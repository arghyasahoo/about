// Header and Macros
#include "headers.h"
#include "macros.h"
#include "render.h"


// required functions
void validateFileName(char *filepath);
bool isValidFilename(char *filepath);
char *getFileType(mode_t st_mode);
char *getFileOwner(uid_t uid);
char *getFileGroup(gid_t gid);
void getFilePerms(char *filepath, char *file_perms);
void getLast(char *timeStr, int len, struct timespec *ts);
void getFileContent(char *filepath, char *content,unsigned long tcol);
char **getFileExtensionAndCharset(char *filepath, struct magic_set *magic);
unsigned long getTerminalSize(void);
int timespec2str(char *buf, uint len, struct timespec *ts);
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
	magic_load(magic, NULL);

	unsigned long inode_no, size, tcol;
	tcol = getTerminalSize();

	char *filename = (char *)malloc(sizeof(char) * FILE_NAME_MAX_LEN);
	char *file_extension = (char *)malloc(sizeof(char) * FILE_EXT_MAX_LEN);
	char *charset = (char *)malloc(sizeof(char) * FILE_CSET_MAX_LEN);
	char *file_inode = (char *)malloc(sizeof(char) * MAX_INODE_DIGITS);
	char *file_size = (char *)malloc(sizeof(char) * FILE_EXT_MAX_LEN);
	char *file_owner = (char *)malloc(sizeof(char) * MAX_USER_LEN);
	char *file_group = (char *)malloc(sizeof(char) * MAX_USER_LEN);
	char *last_accessed = (char *)malloc(sizeof(char) * TIME_LEN);
	char *last_modified = (char *)malloc(sizeof(char) * TIME_LEN);
	char *abspath = (char *)malloc(sizeof(char) * FILE_PATH_MAX_LEN);
	char *content = (char *)malloc(sizeof(char) * tcol-22);
	char *file_type = (char *)malloc(sizeof(char) * MAX_FILE_TYPE_LEN);
	char **file_magic = (char **)malloc(sizeof(char *) * 2);
	char *file_perms = (char *)malloc(sizeof(char ) * 20);

	// store results
	filename = basename(filepath);
	inode_no = statbuf->st_ino;
	size = statbuf->st_size;
	file_type = getFileType(statbuf->st_mode);
	file_magic = getFileExtensionAndCharset(filepath, magic);
	file_extension = file_magic[0];
	charset = file_magic[1];
	abspath = realpath(filepath, NULL);
	getFilePerms(filepath, file_perms);
	file_owner = getFileOwner(statbuf->st_uid);
	file_group = getFileGroup(statbuf->st_gid);
	getLast(last_accessed, TIME_LEN, &(statbuf->st_atim));
	getLast(last_modified, TIME_LEN, &(statbuf->st_mtim));
	getFileContent(filepath, content, tcol);

	snprintf(file_size, FILE_EXT_MAX_LEN, "%ld", size);
	snprintf(file_inode, MAX_INODE_DIGITS, "%ld", inode_no);

	// render details into terminal
	render("Filename", filename, tcol);
	render("Inode Number", file_inode, tcol);
	render("Type", file_type, tcol);
	render("Size", file_size, tcol);
	render("Extension", file_extension, tcol);
	render("Charset", charset, tcol);
	render("Absolute Path", abspath, tcol);
	render("Owner", file_owner, tcol);
	render("Group", file_group, tcol);
	render("Permission", file_perms, tcol);
	render("Last Accessed", last_accessed, tcol);
	render("Last Modified", last_modified, tcol);
	render("Content", content, tcol);
	finish_render(tcol);

	// free allocated memory
	free(filepath);
	free(statbuf);
	free(magic);
	// free(filename);
	free(file_extension);
	free(charset);
	free(file_owner);
	free(file_group);
	free(last_accessed);
	free(last_modified);
	free(abspath);
	free(file_perms);
}

void validateFileName(char *filepath)
{
	if (strlen(filepath) == 0)
	{
		error("[-] Filename cannot be EMPTY", 1);
	}
	else if (access(filepath, F_OK))
	{
		error("[-] File does not exist", 2);
	}
}

char *getFileType(mode_t st_mode)
{
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

void getFilePerms(char *filepath, char *file_perms)
{
	if (!access(filepath, R_OK))
		strcat(file_perms, "  read");
	if (!access(filepath, W_OK))
		strcat(file_perms, "  write");
	if (!access(filepath, X_OK))
		strcat(file_perms, "  execute");
}

char *getFileOwner(uid_t uid)
{
	struct passwd *user;
	user = getpwuid(uid);
	return user->pw_name;
}

char *getFileGroup(gid_t gid)
{
	struct group *group;
	group = getgrgid(gid);
	return group->gr_name;
}

void getLast(char *timeStr, int len, struct timespec *ts)
{
	int status = timespec2str(timeStr, len, ts);

	if (status != 0)
		error("[-] Conversion from TIMESPEC to STRING failed", status);
}

void getFileContent(char *filepath, char *content, unsigned long tcol) {
	FILE *fp = fopen(filepath, "r");
	char ch;
	int count=0;

	while ((ch = fgetc(fp)) != '\n' && count < tcol-22) {
		content[count++] = ch;
	}
}

char **getFileExtensionAndCharset(char *filepath, struct magic_set *magic)
{
	const char *file_magic = (char *)malloc(sizeof(char) * FILE_CSET_MAX_LEN);
	char **ret = (char **)malloc(sizeof(char *) * FILE_CSET_MAX_LEN);
	ret[0] = (char *)malloc(sizeof(char) * FILE_EXT_MAX_LEN);
	ret[1] = (char *)malloc(sizeof(char) * FILE_EXT_MAX_LEN);
	bool isExt = false, isCset = false;
	int j;

	file_magic = magic_file(magic, filepath);

	for (int i = 0; i < strlen(file_magic); ++i)
	{
		if (file_magic[i] == ';')
			isExt = false;
		else if (file_magic[i] == '\0')
			isCset = false;

		if (isExt)
			ret[0][j++] = file_magic[i];
		else if (isCset)
			ret[1][j++] = file_magic[i];

		if (file_magic[i] == '/')
		{
			isExt = true;
			j = 0;
		}
		else if (file_magic[i] == '=')
		{
			isCset = true;
			j = 0;
		}
	}

	return ret;
}

unsigned long getTerminalSize(void) {
	struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}

int timespec2str(char *buf, uint len, struct timespec *ts)
{
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

void error(char *err_msg, int exitCode)
{
	perror(err_msg);
	exit(exitCode);
}
