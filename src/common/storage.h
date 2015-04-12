#ifndef COMMON_STORAGE_H
#define COMMON_STORAGE_H

#include <map>
#include <string>
#include "common/logger.h"
#include "common/types.h"


namespace Airwave {


class Storage {
public:
	class Prefix {
	public:
		Prefix();
		bool isNull() const;

		std::string name() const;
		bool setName(const std::string& name);

		std::string path() const;
		bool setPath(const std::string& path);

		Prefix next() const;
		bool operator!() const;

	private:
		friend class Storage;
		using PrefixMap = std::map<std::string, std::string>;

		Storage* storage_;
		PrefixMap::iterator it_;

		Prefix(Storage* storage, PrefixMap::iterator it);
	};

	class Loader {
	public:
		Loader();
		bool isNull() const;

		std::string name() const;
		bool setName(const std::string& name);

		std::string path() const;
		bool setPath(const std::string& path);

		Loader next() const;
		bool operator!() const;

	private:
		friend class Storage;
		using LoaderMap = std::map<std::string, std::string>;

		Storage* storage_;
		LoaderMap::iterator it_;

		Loader(Storage* storage, LoaderMap::iterator it);
	};


	struct LinkInfo {
		std::string target;
		std::string prefix;
		std::string loader;
		LogLevel level;
	};

	class Link {
	public:
		Link();
		bool isNull() const;

		std::string path() const;
		bool setPath(const std::string& path);

		std::string target() const;
		bool setTarget(const std::string& path);

		std::string prefix() const;
		bool setPrefix(const std::string& prefix);

		std::string loader() const;
		bool setLoader(const std::string& loader);

		LogLevel logLevel() const;
		void setLogLevel(LogLevel level);

		Link next() const;
		bool operator!() const;

	private:
		friend class Storage;
		using LinkMap = std::map<std::string, LinkInfo>;

		Storage* storage_;
		LinkMap::iterator it_;

		Link(Storage* storage, LinkMap::iterator it);
	};

	Storage();
	~Storage();

	bool reload();
	bool save();

	std::string logSocketPath() const;
	void setLogSocketPath(const std::string& path);

	std::string binariesPath() const;
	void setBinariesPath(const std::string& path);

	LogLevel defaultLogLevel() const;
	void setDefaultLogLevel(LogLevel level);

	Prefix prefix(const std::string& name = std::string());
	Prefix createPrefix(const std::string& name, const std::string& path);
	bool removePrefix(Prefix prefix);

	Loader loader(const std::string& name = std::string());
	Loader createLoader(const std::string& name, const std::string& path);
	bool removeLoader(Loader loader);

	Link link(const std::string& path = std::string());

	Link createLink(const std::string& path, const std::string& target,
			const std::string& prefix, const std::string& loader);

	bool removeLink(Link link);

private:
	friend class Prefix;
	friend class Loader;
	friend class Link;

	bool isChanged_;

	std::string storageFilePath_;
	std::string logSocketPath_;
	std::string binariesPath_;
	LogLevel defaultLogLevel_;

	std::map<std::string, std::string> prefixByName_;
	std::map<std::string, std::string> loaderByName_;
	std::map<std::string, LinkInfo> linkByPath_;
};


} // namespace Airwave


#endif // COMMON_STORAGE_H
