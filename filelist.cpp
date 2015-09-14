#include "filelist.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "camera.h"

namespace Lyli {

class FileListEntry::Impl {
public:
	// camera containing the file
	Camera *camera;
	// directory base name
	const std::string dirBase;
	// directory id
	const int dirId;
	// file base name
	const std::string fileBase;
	// the counter for file name
	const int fileId;
	// a sha1 hash (stored as binary)
	const Sha1Array sha1;
	// a time the picture was taken
	const std::time_t time;
	
	mutable std::string pathBase;
	
	Impl(Camera *camera, const std::string &dirBase, int dirId, const std::string &fileBase, int fileId, const Sha1Array &sha1, const std::time_t &time);
	
	std::string &getPathBase() const;
};

FileListEntry::Impl::Impl(Camera *camera_, const std::string &dirBase_, int dirId_, const std::string &fileBase_, int fileId_, const Sha1Array &sha1_, const std::time_t &time_) :
	camera(camera_), dirBase(dirBase_), dirId(dirId_), fileBase(fileBase_), fileId(fileId_), sha1(sha1_), time(time_)
{
	
}

std::string &FileListEntry::Impl::getPathBase() const {
	if (pathBase.empty()) {
		std::stringstream ss;
		ss << "I:\\DCIM\\";
		ss << dirId << dirBase << "\\";
		ss << fileBase << std::setfill ('0') << std::setw (4) << fileId;
		pathBase = ss.str();
	}
	return pathBase;
}

FileListEntry::FileListEntry() : pimpl(0)
{
	
}

FileListEntry::~FileListEntry()
{
	if (pimpl != nullptr) {
		delete pimpl;
	}
}

FileListEntry::FileListEntry(Camera *camera, const std::string &dirBase, int dirId, const std::string &fileBase, int fileId, const Sha1Array &sha1, const std::time_t &time) :
	pimpl(new Impl(camera, dirBase, dirId, fileBase, fileId, sha1, time))
{
	
}

FileListEntry::FileListEntry(const FileListEntry &other) :
	pimpl(new Impl(other.pimpl->camera, other.pimpl->dirBase, other.pimpl->dirId, other.pimpl->fileBase, other.pimpl->fileId, other.pimpl->sha1, other.pimpl->time)) {
	
}

FileListEntry &FileListEntry::operator=(const FileListEntry &other)
{
	if (this == &other) {
		return *this;
	}
	
	FileListEntry tmp(other);
	std::swap(pimpl, tmp.pimpl);
	
	return *this;
}

FileListEntry::FileListEntry(FileListEntry &&other) noexcept
{
	pimpl = other.pimpl;
	other.pimpl = nullptr;
}

FileListEntry &FileListEntry::operator=(FileListEntry &&other) noexcept
{
	if (this == &other) {
		return *this;
	}
	
	std::swap(pimpl, other.pimpl);
	return *this;
}

std::time_t FileListEntry::getTime() const
{
	return pimpl->time;
}

FilePtr FileListEntry::getImageMetadata() const
{
	std::stringstream ss;
	ss << pimpl->getPathBase() << ".TXT";
	return std::make_shared<File>(pimpl->camera, ss.str());
}

FilePtr FileListEntry::getImageThumbnail() const
{
	std::stringstream ss;
	ss << pimpl->getPathBase() << ".128";
	return std::make_shared<File>(pimpl->camera, ss.str());
}

FilePtr FileListEntry::getImageData() const
{
	std::stringstream ss;
	ss << pimpl->getPathBase() << ".RAW";
	return std::make_shared<File>(pimpl->camera, ss.str());
}

}

template class std::vector<Lyli::FileListEntry>;
