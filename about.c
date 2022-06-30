// Header and Macros

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<magic.h>
#include<limits.h>
#include<libgen.h>
#include<grp.h>
#include<pwd.h>
#include<time.h>

#define NANO 1000000000L
#define FILE_EXT_MAX_LEN 255
#define FILE_NAME_MAX_LEN 255
#define FILE_CSET_MAX_LEN 255
#define FILE_PATH_MAX_LEN 4096
#define MAX_CONTENT_LEN 20
#define MAX_FILE_TYPE_LEN 32
#define MAX_USER_LEN 32
#define TIME_LEN 29


// required functions
void validateFileName(char *filepath);
bool isValidFilename(char *filepath);
char *getFileType(mode_t st_mode);
char *getFileOwner(uid_t uid);
char *getFileGroup(gid_t gid);
void getFilePerms(char *filepath, char **file_perms);
void getLast(char *timeStr, int len, struct timespec *ts);
char *getFileContent(char *filepath);
char **getFileExtensionAndCharset(char *filepath, struct magic_set *magic);
int timespec2str(char *buf, uint len, struct timespec *ts);
void display(char *filename, unsigned long inode_no, char *file_type, char *file_extension, char *charset, unsigned long file_size, char *abspath, char **file_perms, char *file_owner, char *file_group, char *last_accessed, char *last_modified, char *content);
void error(char *err_msg, int exitCode);


// driver code
int main(int argc, char *argv[])
{
	char *filepath = (char *)malloc(sizeof(char) * FILE_PATH_MAX_LEN);

	strncpy(filepath, argv[1], FILE_PATH_MAX_LEN);
	validateFileName(filepath);

	// allocate variables
	struct stat *statbuf = (struct stat *)malloc(sizeof(struct stat));

	if (fstatat(AT_FDCWD, filepath, statbuf, AT_SYMLINK_NOFOLLOW))
		error("[-] Failed to access file information", 1);

	struct magic_set *magic = magic_open(MAGIC_MIME | MAGIC_CHECK);
	magic_load(magic,NULL);

	char *filename = (char *)malloc(sizeof(char) * FILE_NAME_MAX_LEN);
	char *file_extension = (char *)malloc(sizeof(char) * FILE_EXT_MAX_LEN);
	char *charset = (char *)malloc(sizeof(char) * FILE_CSET_MAX_LEN);
	char *file_owner = (char *)malloc(sizeof(char) * MAX_USER_LEN);
	char *file_group = (char *)malloc(sizeof(char) * MAX_USER_LEN);
	char *last_accessed = (char *)malloc(sizeof(char) * TIME_LEN);
	char *last_modified = (char *)malloc(sizeof(char) * TIME_LEN);
	char *abspath = (char *)malloc(sizeof(char) * FILE_PATH_MAX_LEN);
	char *content = (char *)malloc(sizeof(char) * MAX_CONTENT_LEN);
	char *file_type = (char *)malloc(sizeof(char) * MAX_FILE_TYPE_LEN);
	char **file_magic = (char **)malloc(sizeof(char *) * 2);
	char **file_perms = (char **)malloc(sizeof(char *) * 3);
	unsigned long inode_no, file_size;


	// store results
	filename = basename(filepath);
	inode_no = statbuf->st_ino;
	file_size = statbuf->st_size;
	file_type = getFileType(statbuf->st_mode);
	file_magic = getFileExtensionAndCharset(filepath, magic);
	file_extension = file_magic[0];
	charset = file_magic[1];
	abspath = realpath(filepath, NULL);
	getFilePerms(filepath, file_perms);
	file_owner = getFileOwner(statbuf->st_uid);
	file_group = getFileOwner(statbuf->st_gid);
	getLast(last_accessed, TIME_LEN, &(statbuf->st_atim));
	getLast(last_modified, TIME_LEN, &(statbuf->st_mtim));
	content = getFileContent(filepath);


	// display results
	display(filename, inode_no, file_type, file_extension, charset, file_size, abspath, file_perms, file_owner, file_group, last_accessed, last_modified, content);

	// free allocated memory
	free(filepath);
	free(statbuf);
	free(magic);
	// free(filename);
	free(file_extension);
	free(charset);
	free(file_owner);
	// free(file_group);
	free(last_accessed);
	free(last_modified);
	free(abspath);
	free(file_perms);
}

void validateFileName(char *filepath) {
	if (strlen(filepath) == 0) {
		error("[-] Filename cannot be EMPTY", 1);
	} else if (access(filepath, F_OK)) {
		error("[-] File does not exist", 2);
	}
}

char *getFileType(mode_t st_mode) {
	if (S_ISREG(st_mode))
		return "Regular File";
	else if (S_ISDIR(st_mode))
		return "Directory";
	else if (S_ISCHR(st_mode))
		return "Character Device";
	else if (S_ISBLK(st_mode))
		return "Block Device";
	else if (S_ISFIFO(st_mode))
		return "Named Pipe";
	else if (S_ISLNK(st_mode))
		return "Symbolic Link";
	else if (S_ISSOCK(st_mode))
		return "Socket";
	return "Unknown File Type";
}

void getFilePerms(char *filepath, char **file_perms) {
	if (access(filepath, R_OK))
		file_perms[0] = "read";
	if (access(filepath, W_OK))
		file_perms[1] = "write";
	if (access(filepath, X_OK))
		file_perms[2] = "execute";
}

char *getFileOwner(uid_t uid) {
	struct passwd *user;
	user = getpwuid(uid);
	return user->pw_name;
}

char *getFileGroup(gid_t gid) {
	struct group *group;
	group = getgrgid(gid);
	return group->gr_name;
}

void getLast(char *timeStr, int len, struct timespec *ts) {
	int status = timespec2str(timeStr, len, ts);

	if (status != 0)
        error("[-] Conversion from TIMESPEC to STRING failed", status);
}

char *getFileContent(char *filepath) {
	return "demo content";
}

void display(char *filename, unsigned long inode_no, char *file_type, char *file_extension, char *charset, unsigned long file_size, char *abspath, char **file_perms, char *file_owner, char *file_group, char *last_accessed, char *last_modified, char *content) {
	printf("\nFilename: %s", filename);
	printf("\nFile Type: %s", file_type);
	printf("\nSize: %lu", file_size);
	printf("\nExtention: %s", file_extension);
	printf("\nCharset: %s", charset);
	printf("\nAbsolute Path: %s", abspath);
	printf("\nPermissions: %s %s %s", file_perms[0], file_perms[1], file_perms[2]);	// make a better print
	printf("\nOwner: %s", file_owner);
	printf("\nGroup: %s", file_group);
	printf("\nLast Accessed: %s", last_accessed);
	printf("\nLast Modified: %s", last_modified);
	printf("\nInode Number: %lu", inode_no);
	printf("\n");
}

char **getFileExtensionAndCharset(char *filepath, struct magic_set *magic) {
	char *file_magic = (char *)malloc(sizeof(char) * FILE_CSET_MAX_LEN);
	char **ret = (char **)malloc(sizeof(char *) * FILE_CSET_MAX_LEN);
	ret[0] = (char *)malloc(sizeof(char ) * FILE_EXT_MAX_LEN);
	ret[1] = (char *)malloc(sizeof(char ) * FILE_EXT_MAX_LEN);
	bool isExt=false, isCset=false;
	int j;
	
	file_magic = magic_file(magic, filepath);

	for (int i=0; i<strlen(file_magic); ++i) {
		if (file_magic[i] == ';')
			isExt = false;
		else if (file_magic[i] == '\0')
			isCset = false;

		if (isExt)
			ret[0][j++] = file_magic[i];
		else if (isCset)
			ret[1][j++] = file_magic[i];

		
		if (file_magic[i] == '/') {
			isExt = true;
			j = 0;
		}
		else if (file_magic[i] == '=') {
			isCset = true;
			j = 0;
		}
	}

	return ret;

}

int timespec2str(char *buf, uint len, struct timespec *ts) {
    int ret;
    struct tm t;

    tzset();
    if (localtime_r(&(ts->tv_sec), &t) == NULL)
        return 1;

    ret = strftime(buf, len, "%F %T", &t);
    if (ret == 0)
        return 2;
    len -= ret - 1;

    ret = puts(&buf[strlen(buf)]);
    if (ret >= len)
        return 3;

    return 0;
}

void error(char *err_msg, int exitCode) {
	perror(err_msg);
	exit(exitCode);
}