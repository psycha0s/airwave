#include "storage.h"

#include <fstream>
#include "common/config.h"
#include "common/filesystem.h"
#include "common/json.h"


namespace Airwave {


Storage::Storage() :
	isChanged_(false)
{
	reload();
}


Storage::~Storage()
{
	save();
}


bool Storage::reload()
{
	storageFilePath_.clear();
	logSocketPath_.clear();
	binariesPath_.clear();
	prefixByName_.clear();
	loaderByName_.clear();
	linkByPath_.clear();

	// Add default WINE prefix
	std::string name = "default";
	std::string path = FileSystem::realPath("~") + "/.wine";
	prefixByName_.emplace(makePair(name, path));

	// Add default WINE loader
	name = "default";
	path = FileSystem::fullNameFromPath("wine");
	loaderByName_.emplace(makePair(name, path));

	// Initialize default values
	binariesPath_ = INSTALL_PREFIX "/bin";

	const char* string = getenv("TMPDIR");
	std::string tempPath = string ? string : "/tmp";
	logSocketPath_ = tempPath + "/" PROJECT_NAME ".sock";

	defaultLogLevel_ = LogLevel::kTrace;

	// Find and read a configuration file
	string = getenv("XDG_CONFIG_PATH");
	std::string configPath = string ? string : std::string();
	if(configPath.empty())
		configPath = "~/.config";

	configPath = FileSystem::realPath(configPath);
	if(configPath.back() != '/')
		configPath += '/';

	if(!FileSystem::isDirExists(configPath))
		return false;

	storageFilePath_ = configPath + PROJECT_NAME "/" PROJECT_NAME ".conf";

	std::ifstream file(storageFilePath_);
	if(!file.is_open())
		return false;

	Json::Value root;
	Json::Reader reader;
	if(!reader.parse(file, root, false))
		return false;

	// Load general variables
	auto value = root["binaries_path"];
	if(!value.isNull())
		binariesPath_ = value.asString();

	value = root["log_socket_path"];
	if(!value.isNull())
		logSocketPath_ = value.asString();

	value = root["default_log_level"];
	if(!value.isNull()) {
		defaultLogLevel_ = static_cast<LogLevel>(value.asInt());
		if(defaultLogLevel_ < LogLevel::kQuiet || defaultLogLevel_ > LogLevel::kFlood)
			defaultLogLevel_ = LogLevel::kTrace;
	}

	// Load prefixes
	Json::Value prefixes = root["prefixes"];
	for(uint i = 0; i < prefixes.size(); ++i) {
		Json::Value prefix = prefixes[i];

		name = prefix["name"].asString();
		path = prefix["path"].asString();

		if(name.empty() || path.empty())
			continue;

		prefixByName_.emplace(makePair(name, path));
	}

	// Load loaders
	Json::Value loaders = root["loaders"];
	for(uint i = 0; i < loaders.size(); ++i) {
		Json::Value loader = loaders[i];

		name = loader["name"].asString();
		path = loader["path"].asString();

		if(name.empty() || path.empty())
			continue;

		loaderByName_.emplace(makePair(name, path));
	}

	// Load links
	Json::Value links = root["links"];
	for(uint i = 0; i < links.size(); ++i) {
		Json::Value link = links[i];

		LinkInfo info;
		info.target = link["target"].asString();

		info.loader = link["loader"].asString();
		if(info.loader.empty())
			info.loader = "default;";

		info.prefix = link["prefix"].asString();
		if(info.prefix.empty())
			info.prefix = "default;";

		auto value = link["log_level"];
		if(value.isNull()) {
			info.level = LogLevel::kDefault;
		}
		else {
			info.level = static_cast<LogLevel>(value.asInt());
			if(info.level < LogLevel::kDefault || info.level > LogLevel::kFlood)
				info.level = LogLevel::kDefault;
		}

		path = link["path"].asString();
		linkByPath_.emplace(makePair(path, info));
	}

	return true;
}


bool Storage::save()
{
	if(!isChanged_)
		return true;

	std::size_t pos = storageFilePath_.rfind('/');
	if(pos != std::string::npos) {
		std::string dir = storageFilePath_.substr(0, pos);
		if(!FileSystem::makePath(dir))
			return false;
	}

	std::ofstream file(storageFilePath_, std::ios::out | std::ios::trunc);

	if(!file.is_open())
		return false;

	Json::Value root;
	root["binaries_path"] = binariesPath_;
	root["log_socket_path"] = logSocketPath_;
	root["default_log_level"] = static_cast<int>(defaultLogLevel_);

	Json::Value prefixes(Json::arrayValue);
	for(auto it : prefixByName_) {
		if(it.first == "default")
			continue;

		Json::Value prefix;
		prefix["name"] = it.first;
		prefix["path"] = it.second;

		prefixes.append(prefix);
	}

	root["prefixes"] = prefixes;

	Json::Value loaders(Json::arrayValue);
	for(auto it : loaderByName_) {
		if(it.first == "default")
			continue;

		Json::Value loader;
		loader["name"] = it.first;
		loader["path"] = it.second;

		loaders.append(loader);
	}

	root["loaders"] = loaders;

	Json::Value links(Json::arrayValue);
	for(auto& it : linkByPath_) {
		Json::Value link;
		link["path"]   = it.first;
		link["loader"] = it.second.loader;
		link["prefix"] = it.second.prefix;
		link["target"] = it.second.target;
		link["log_level"] = static_cast<int>(it.second.level);

		links.append(link);
	}

	root["links"] = links;

	Json::StyledWriter writer;
	file << writer.write(root);

	isChanged_ = false;
	return true;
}


std::string Storage::logSocketPath() const
{
	return logSocketPath_;
}


void Storage::setLogSocketPath(const std::string& path)
{
	logSocketPath_ = path;
	isChanged_ = true;
}


std::string Storage::binariesPath() const
{
	return binariesPath_;
}


void Storage::setBinariesPath(const std::string& path)
{
	binariesPath_ = path;
	isChanged_ = true;
}


LogLevel Storage::defaultLogLevel() const
{
	return defaultLogLevel_;
}


void Storage::setDefaultLogLevel(LogLevel level)
{
	defaultLogLevel_ = level;
	isChanged_ = true;
}


Storage::Prefix Storage::prefix(const std::string& name)
{
	if(name.empty())
		return Prefix(this, prefixByName_.begin());

	return Prefix(this, prefixByName_.find(name));
}


Storage::Prefix Storage::createPrefix(const std::string& name, const std::string& path)
{
	auto it = prefixByName_.find(name);
	if(it != prefixByName_.end())
		return Prefix();

	auto result = prefixByName_.emplace(name, path);
	isChanged_ = true;
	return Prefix(this, result.first);
}


bool Storage::removePrefix(Storage::Prefix prefix)
{
	if(!prefix || prefix.name() == "default")
		return false;

	prefixByName_.erase(prefix.it_);
	isChanged_ = true;
	return true;
}


Storage::Loader Storage::loader(std::string const& name)
{
	if(name.empty())
		return Loader(this, loaderByName_.begin());

	return Loader(this, loaderByName_.find(name));
}


Storage::Loader Storage::createLoader(const std::string& name, const std::string& path)
{
	auto it = loaderByName_.find(name);
	if(it != loaderByName_.end())
		return Loader();

	auto result = loaderByName_.emplace(name, path);
	isChanged_ = true;
	return Loader(this, result.first);
}


bool Storage::removeLoader(Storage::Loader loader)
{
	if(!loader || loader.name() == "default")
		return false;

	loaderByName_.erase(loader.it_);
	isChanged_ = true;
	return true;
}


Storage::Link Storage::link(std::string const& path)
{
	if(path.empty())
		return Link(this, linkByPath_.begin());

	return Link(this, linkByPath_.find(path));
}


Storage::Link Storage::createLink(const std::string& path, const std::string& target,
		const std::string& prefix, const std::string& loader)
{
	auto it = linkByPath_.find(path);
	if(it != linkByPath_.end())
		return Link();

	auto prefixIt = prefixByName_.find(prefix);
	if(prefixIt == prefixByName_.end())
		return Link();

	auto loaderIt = loaderByName_.find(loader);
	if(loaderIt == loaderByName_.end())
		return Link();

	LinkInfo info;
	info.target = target;
	info.prefix = prefix;
	info.loader = loader;
	info.level  = LogLevel::kDefault;

	auto result = linkByPath_.emplace(makePair(path, info));
	if(!result.second)
		return Link();

	isChanged_ = true;
	return Link(this, result.first);
}


bool Storage::removeLink(Storage::Link link)
{
	if(!link)
		return false;

	linkByPath_.erase(link.it_);
	isChanged_ = true;
	return true;
}


Storage::Prefix::Prefix() :
	storage_(nullptr)
{
}


Storage::Prefix::Prefix(Storage* storage, PrefixMap::iterator it) :
	storage_(storage),
	it_(it)
{
}


bool Storage::Prefix::isNull() const
{
	return !storage_ || it_ == storage_->prefixByName_.end();
}


std::string Storage::Prefix::name() const
{
	if(isNull())
		return std::string();

	return it_->first;
}


bool Storage::Prefix::setName(const std::string& name)
{
	if(isNull())
		return false;

	std::string oldName = it_->first;
	std::string path = it_->second;
	auto result = storage_->prefixByName_.emplace(makePair(name, path));
	if(!result.second)
		return false;

	for(auto& it : storage_->linkByPath_) {
		if(it.second.prefix == oldName)
			it.second.prefix = name;
	}

	storage_->prefixByName_.erase(it_);
	storage_->isChanged_ = true;
	it_ = result.first;
	return true;
}


std::string Storage::Prefix::path() const
{
	if(isNull())
		return std::string();

	return it_->second;
}


bool Storage::Prefix::setPath(const std::string& path)
{
	if(isNull())
		return false;

	if(path == it_->second)
		return true;

	it_->second = path;
	storage_->isChanged_ = true;
	return true;
}


Storage::Prefix Storage::Prefix::next() const
{
	if(storage_ && it_ != storage_->prefixByName_.end()) {
		auto nextIt = it_;
		return Prefix(storage_, ++nextIt);
	}

	return Prefix();
}


bool Storage::Prefix::operator!() const
{
	return isNull();
}


Storage::Loader::Loader() :
	storage_(nullptr)
{
}


Storage::Loader::Loader(Storage* storage, LoaderMap::iterator it) :
	storage_(storage),
	it_(it)
{
}


bool Storage::Loader::isNull() const
{
	return !storage_ || it_ == storage_->loaderByName_.end();
}


std::string Storage::Loader::name() const
{
	if(isNull())
		return std::string();

	return it_->first;
}


bool Storage::Loader::setName(const std::string& name)
{
	if(isNull())
		return false;

	std::string oldName = it_->first;
	std::string path = it_->second;
	auto result = storage_->loaderByName_.emplace(makePair(name, path));
	if(!result.second)
		return false;

	for(auto& it : storage_->linkByPath_) {
		if(it.second.loader == oldName)
			it.second.loader = name;
	}

	storage_->loaderByName_.erase(it_);
	storage_->isChanged_ = true;
	it_ = result.first;
	return true;
}


std::string Storage::Loader::path() const
{
	if(isNull())
		return std::string();

	return it_->second;
}


bool Storage::Loader::setPath(const std::string& path)
{
	if(isNull())
		return false;

	if(path == it_->second)
		return true;

	it_->second = path;
	storage_->isChanged_ = true;
	return true;
}


Storage::Loader Storage::Loader::next() const
{
	if(storage_ && it_ != storage_->loaderByName_.end()) {
		auto nextIt = it_;
		return Loader(storage_, ++nextIt);
	}

	return Loader();
}


bool Storage::Loader::operator!() const
{
	return isNull();
}


Storage::Link::Link() :
	storage_(nullptr)
{
}


Storage::Link::Link(Storage* storage, LinkMap::iterator it) :
	storage_(storage),
	it_(it)
{
}


bool Storage::Link::isNull() const
{
	return !storage_ || it_ == storage_->linkByPath_.end();
}


std::string Storage::Link::path() const
{
	if(isNull())
		return std::string();

	return it_->first;
}


bool Storage::Link::setPath(const std::string& path)
{
	if(isNull())
		return false;

	if(path == it_->first)
		return true;

	auto it = storage_->linkByPath_.find(path);
	if(it != storage_->linkByPath_.end())
		return false;

	LinkInfo info = it_->second;
	storage_->linkByPath_.erase(it_);
	it_ = storage_->linkByPath_.emplace_hint(it, makePair(path, info));

	storage_->isChanged_ = true;
	return true;
}


std::string Storage::Link::target() const
{
	if(isNull())
		return std::string();

	return it_->second.target;
}


bool Storage::Link::setTarget(const std::string& path)
{
	if(isNull())
		return false;

	if(path == it_->second.target)
		return true;

	it_->second.target = path;
	storage_->isChanged_ = true;
	return true;
}


std::string Storage::Link::prefix() const
{
	if(isNull())
		return std::string();

	return it_->second.prefix;
}


bool Storage::Link::setPrefix(const std::string& prefix)
{
	if(isNull())
		return false;

	if(prefix == it_->second.prefix)
		return true;

	it_->second.prefix = prefix;
	storage_->isChanged_ = true;
	return true;
}


std::string Storage::Link::loader() const
{
	if(isNull())
		return std::string();

	return it_->second.loader;
}


bool Storage::Link::setLoader(const std::string& loader)
{
	if(isNull())
		return false;

	if(loader == it_->second.loader)
		return true;

	it_->second.loader = loader;
	storage_->isChanged_ = true;
	return true;
}


LogLevel Storage::Link::logLevel() const
{
	if(isNull())
		return LogLevel::kDefault;

	return it_->second.level;
}


void Storage::Link::setLogLevel(LogLevel level)
{
	if(!isNull() && level != it_->second.level) {
		it_->second.level = level;
		storage_->isChanged_ = true;
	}
}


Storage::Link Storage::Link::next() const
{
	if(storage_ && it_ != storage_->linkByPath_.end()) {
		auto nextIt = it_;
		return Link(storage_, ++nextIt);
	}

	return Link();
}


bool Storage::Link::operator!() const
{
	return isNull();
}


} // namespace Airwave
