#include "linksmodel.h"


using Airwave::LinkManager;


LinkItem::LinkItem(const QString& pluginPath, const QString& bridgePath,
		const QString& arch) :
	pluginPath_(pluginPath),
	bridgePath_(bridgePath),
	architecture_(arch)
{
}


QString LinkItem::pluginPath() const
{
	return pluginPath_;
}


QString LinkItem::bridgePath() const
{
	return bridgePath_;
}


QString LinkItem::architecture() const
{
	return architecture_;
}


LinksModel::LinksModel(QObject* parent) :
	GenericTreeModel<LinkItem>(new LinkItem(), parent)
{
	reload();
}


int LinksModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 3;
}


QVariant LinksModel::data(const QModelIndex& index, int role) const
{
	if(index.isValid()) {
		LinkItem* item = indexToItem(index);

		if(role == Qt::DisplayRole) {
			if(index.column() == 0) {
				return item->pluginPath();
			}
			else if(index.column() == 1) {
				return item->bridgePath();
			}
			else if(index.column() == 2) {
				return item->architecture();
			}
		}
	}

	return QVariant();
}


QVariant LinksModel::headerData(int section, Qt::Orientation orientation,
		int role) const
{
	Q_UNUSED(orientation);

	if(role == Qt::DisplayRole) {
		if(section == 0) {
			return tr("VST plugin path");
		}
		else if(section == 1) {
			return tr("VST bridge path");
		}
		else if(section == 2) {
			return tr("Arch");
		}
	}

	return QVariant();
}


bool LinksModel::setLink(const QString& pluginPath, const QString& bridgePath)
{
//	ModuleInfo::Arch bridgeArch = info_.getArch(bridgePath);
//	ModuleInfo::Arch pluginArch = info_.getArch(pluginPath);

//	if(bridgeArch != pluginArch || bridgeArch == ModuleInfo::kArchUnknown)
//		return false;

	linkManager_.bind(bridgePath.toStdString(), pluginPath.toStdString());
	reload();
	return true;
}


bool LinksModel::updateLink(const QString& bridgePath, const QString& newPath)
{
	if(linkManager_.rebind(bridgePath.toStdString(), newPath.toStdString())) {
		reload();
		return true;
	}

	return false;
}


bool LinksModel::removeLink(const QString& bridgePath)
{
	if(linkManager_.unbind(bridgePath.toStdString())) {
		reload();
		return true;
	}

	return false;
}


void LinksModel::reload()
{
	clear();

	for(const std::string& bridgePath : linkManager_.boundBridges()) {
		std::string pluginPath = linkManager_.pluginPath(bridgePath);
		ModuleInfo::Arch arch = info_.getArch(bridgePath.c_str());
		QString architecture;
		if(arch == ModuleInfo::kArch32) {
			architecture = tr("32-bit");
		}
		else if(arch == ModuleInfo::kArch64) {
			architecture = tr("64-bit");
		}
		else {
			architecture = tr("unknown");
		}

		LinkItem* item = new LinkItem(pluginPath.c_str(), bridgePath.c_str(),
				architecture);

		rootItem()->insertChild(item);
	}
}
