#if !defined __FSUTIL_H__
#define __FSUTIL_H__

boost::filesystem::path home_dir();

// returns a list of all the files in a directory
// does *not* return any subdirectories
void list_directory(const boost::filesystem::path&, std::vector<std::string>& files);

bool file_to_string(const boost::filesystem::path& path, std::string& str);
bool file_to_strings(const boost::filesystem::path& path, std::vector<std::string>& s);

#endif
