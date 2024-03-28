#define FUSE_USE_VERSION 30
#define MAX_NAME_LENGTH 100
#define MAX_FILE_LENGHT (1 << 10)
#define MAX_ENTRIES (1 << 8)
#define FOLDER_ST_MODE __S_IFDIR | 0777
#define FILE_ST_MODE __S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO  //__S_IFREG | 0644

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

struct File {
	char name[MAX_NAME_LENGTH];
	char content[MAX_FILE_LENGHT];
	int size;
	time_t last_access;
	time_t last_modification;
};

struct Directory {
	char name[MAX_NAME_LENGTH];
	struct File **files;
	int numFiles;
	time_t last_access;
	time_t last_modification;
};

struct Master {
	struct Directory **subdirectories;  // Assuming a directory can have up to 10 subdirectories
	int numSubdirectories;
	struct File **files;
	int numFiles;
};

struct Master *root;
char filename[MAX_NAME_LENGTH + 8];


int fisopfs_utimens(const char *path, const struct timespec tv[2]);

int
getEntries()
{
	// devuelve la cantidad de carpetas y archivos en todo el sistema
	int entries = 0;
	for (int i = 0; i < root->numSubdirectories; i++) {
		entries += root->subdirectories[i]->numFiles;
	}
	// root cuenta como entry
	return entries + root->numSubdirectories + root->numFiles + 1;
}

struct Directory *
createDir(const char *path, int new)
{
	printf("[debug] createDir - path: %s\n", path);

	struct Directory *dir =
	        (struct Directory *) malloc(sizeof(struct Directory));
	// memset(root->subdirectories[i]->files, 0, sizeof(root->subdirectories[i]->files));
	strncpy(dir->name, path, sizeof(dir->name) - 1);
	dir->name[sizeof(dir->name) - 1] = '\0';  // Ensure null-termination
	dir->numFiles = 0;
	dir->files = NULL;

	if (new) {
		// flag new: si un dir es creado desde el load, no se actualizan sus fechas.
		struct timespec times[2];
		clock_gettime(CLOCK_REALTIME,
		              &times[1]);  // Obtener el tiempo actual
		dir->last_access = times[1].tv_sec;
		dir->last_modification = times[1].tv_sec;
	}
	return dir;
}

void
addDir(const char *name, int new)
{
	printf("[debug] addDir: %s in root with %d dirs\n",
	       name,
	       root->numSubdirectories);
	root->numSubdirectories++;
	struct Directory **newArr = (struct Directory **) malloc(
	        sizeof(struct Directory *) * (root->numSubdirectories));
	for (int i = 0; i < root->numSubdirectories - 1; i++) {
		newArr[i] = root->subdirectories[i];
	}
	struct Directory *d = createDir(name, new);
	newArr[root->numSubdirectories - 1] = d;
	if (root->numSubdirectories != 1)
		free(root->subdirectories);
	root->subdirectories = newArr;
	printf("[debug] addDir: %s\n", d->name);
}


int
delDir(struct Directory *d)
{
	printf("[debug] delDir\n");
	int indexToDelete = -1;
	for (int i = 0; i < root->numSubdirectories; ++i) {
		if (root->subdirectories[i] == d) {
			indexToDelete = i;
			break;
		}
	}
	if (indexToDelete == -1)
		return -EAGAIN;
	free(root->subdirectories[indexToDelete]);
	for (size_t i = indexToDelete; i < root->numSubdirectories - 1; ++i) {
		root->subdirectories[i] = root->subdirectories[i + 1];
	}

	root->subdirectories = (struct Directory **) realloc(
	        root->subdirectories,
	        (root->numSubdirectories - 1) * sizeof(struct Directory *));
	--root->numSubdirectories;
	return 0;
}

void
replaceName(char *oldName, const char *newName)
{
	printf("[debug] replaceName\n");
	strncpy(oldName, newName, MAX_NAME_LENGTH - 1);
	oldName[MAX_NAME_LENGTH - 1] = '\0';
}

struct File *
createFile(const char *name)
{
	printf("[debug] createFile\n");
	struct File *newFile = (struct File *) malloc(sizeof(struct File));
	memset(newFile->content,
	       0,
	       sizeof(newFile->content));  // Optional: Initialize content to zeros
	newFile->size = 0;
	strncpy(newFile->name, name, sizeof(newFile->name) - 1);  // Copy the name
	newFile->name[sizeof(newFile->name) - 1] = '\0';
	printf("[debug] createFile (end)\n");

	struct timespec times[2];
	clock_gettime(CLOCK_REALTIME, &times[1]);  // Obtener el tiempo actual
	newFile->last_access = times[1].tv_sec;
	newFile->last_modification = times[1].tv_sec;
	return newFile;
}

void
addFile(const char *name, int *num_files, struct File ***arr)
{
	printf("[debug] addFile: %s in dir with %d files\n", name, *num_files);
	++(*num_files);
	struct File **newArr =
	        (struct File **) malloc(sizeof(struct File *) * (*num_files));
	for (int i = 0; i < (*num_files) - 1; i++) {
		newArr[i] = (*arr)[i];
	}
	struct File *f = createFile(name);
	newArr[(*num_files) - 1] = f;
	if (*num_files != 1)
		free(*arr);
	*arr = newArr;
	printf("[debug] addFile: %s\n", f->name);
}


void
saveDir(FILE *archivo, struct Directory *dir)
{
	printf("[debug] saveDir\n");
	fwrite(&dir->name, MAX_NAME_LENGTH, 1, archivo);
	fwrite(&dir->numFiles, sizeof(int), 1, archivo);
	fwrite(&dir->last_access, sizeof(time_t), 1, archivo);
	fwrite(&dir->last_modification, sizeof(time_t), 1, archivo);
	printf("[debug] saveDir- name: %s, files: %d\n", dir->name, dir->numFiles);
	for (int i = 0; i < dir->numFiles; i++) {
		if (dir->files[i]) {
			fwrite(dir->files[i], sizeof(struct File), 1, archivo);
			free(dir->files[i]);
		}
	}
	free(dir);
}

void
loadDir(struct Directory *dir, FILE *archivo)
{
	printf("[debug] loadDir\n");
	fread(&dir->name, MAX_NAME_LENGTH, 1, archivo);
	fread(&dir->numFiles, sizeof(int), 1, archivo);
	fread(&dir->last_access, sizeof(time_t), 1, archivo);
	fread(&dir->last_modification, sizeof(time_t), 1, archivo);
	printf("[debug] loadDir- name: %s, files: %d\n", dir->name, dir->numFiles);
	// memset(dir->files, 0, sizeof(dir->files));
	for (int i = 0; i < dir->numFiles; i++) {
		addFile("", &i, &dir->files);
		i--;
		fread(dir->files[i], sizeof(struct File), 1, archivo);
		printf("[debug] loadDir - file: %s\n", dir->files[i]->name);
	}
}

static void
fisopfs_destroy(void *private_data)
{
	printf("[debug] destroy\n");
	FILE *archivo = fopen(filename, "wb");
	if (!archivo) {
		perror("Error opening file for writing");
		return;
	}
	fwrite(&root->numFiles, sizeof(int), 1, archivo);
	printf("[debug] destroy - files: %d\n", root->numFiles);
	for (int i = 0; i < root->numFiles; i++) {
		if (root->files[i]) {
			printf("[debug] files [%d]: %s\n", i, root->files[i]->name);
			fwrite(root->files[i], sizeof(struct File), 1, archivo);
			free(root->files[i]);
		}
	}
	printf("[debug] destroy - dirs: %d\n", root->numSubdirectories);
	fwrite(&root->numSubdirectories, sizeof(int), 1, archivo);
	for (int j = 0; j < root->numSubdirectories; j++) {
		if (root->subdirectories[j])
			saveDir(archivo, root->subdirectories[j]);  // GUARDA
	}
	free(root);
	fclose(archivo);
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] init\n");
	// Allocate memory for the root structure
	root = (struct Master *) malloc(sizeof(struct Master));
	root->numFiles = 0;
	root->numSubdirectories = 0;
	root->subdirectories = NULL;
	root->files = NULL;
	FILE *archivo = fopen(filename, "rb");
	if (!archivo) {
		perror("Error opening file for reading");
		return 0;
	}
	fread(&root->numFiles, sizeof(int), 1, archivo);
	printf("[debug] init (mid1) - files: %d\n", root->numFiles);
	for (int j = 0; j < root->numFiles; j++) {
		addFile("", &j, &root->files);
		j--;
		fread(root->files[j], sizeof(struct File), 1, archivo);
	}
	int numSubdir_temp = 0;
	// si lo cargo directamente en root, addDir se rompe todo pq no coincide con el tamaño del array
	fread(&numSubdir_temp, sizeof(int), 1, archivo);
	printf("[debug] init (mid2) - subd: %d\n", numSubdir_temp);
	for (int i = 0; i < numSubdir_temp; i++) {
		addDir("", 0);
		loadDir(root->subdirectories[i], archivo);
	}
	fclose(archivo);
	printf("[debug] init (final) - files: %d, dirs: %d\n",
	       root->numFiles,
	       root->numSubdirectories);
	return NULL;
}

struct File *
searchFile(const char *file, struct Directory *dir)
{
	printf("[debug] searchFile - file: %s\n", file);
	if (!dir) {  // Asumimos que es root
		if (!root->files)
			return NULL;
		for (int j = 0; j < root->numFiles; j++) {
			if (root->files[j] != NULL) {
				if (strcmp(root->files[j]->name, file) == 0) {
					printf("[debug] searchFile - found "
					       "file in root!\n");
					return root->files[j];
				}
			}
		}
		return NULL;
	} else {
		if (!dir->files)
			return NULL;
		for (int j = 0; j < dir->numFiles; j++) {
			if (dir->files[j] != NULL) {
				if (strcmp(dir->files[j]->name, file) == 0) {
					printf("[debug] searchFile - found "
					       "file in dir!\n");
					return dir->files[j];
				}
			}
		}
		return NULL;
	}
	return NULL;
}

struct Directory *
searchDir(const char *dir)
{
	printf("[debug] searchDir - dir: %s\n", dir);
	for (int j = 0; j < root->numSubdirectories; j++) {
		if (root->subdirectories[j] != NULL) {
			if (strcmp(root->subdirectories[j]->name, dir) == 0) {
				printf("[debug] searchDir - found!\n");
				return root->subdirectories[j];
			}
		}
	}
	return NULL;
}

void
splitPath(const char *path, char *folder, char *file)
{
	printf("[debug] splitPath - path: %s\n", path);
	// PARSEA EL PATH
	// RECIBE 2 PUNTEROS EXTRA ADEMAS DEL PATH
	// EN EL PRIMERO DEVUELVE EL PRIMER DIRECTORIO/ARCHIVO
	// EN EL SEGUNDO DEVUELVE EL ARCHIVO, EN EL CASO DE QUE EL PRIMERO SEA UN DIRECTORIO
	// EL SEGUNDO PUEDE SER NULL SI SE BUSCA UN ARCHIVO EN ROOT
	char pathCopy[256];  // Assuming a maximum length for the path
	strncpy(pathCopy, path, sizeof(pathCopy) - 1);
	pathCopy[sizeof(pathCopy) - 1] = '\0';  // Ensure null-termination

	char *token = strtok(pathCopy, "/");
	int componentCount = 0;

	folder[0] = '\0';  // Initialize folder as an empty string
	file[0] = '\0';    // Initialize file as an empty string

	while (token != NULL) {
		componentCount++;
		if (componentCount == 1) {
			snprintf(folder,
			         255,
			         "%s",
			         token);  // 255 is the size of the destination buffer
		} else if (componentCount == 2) {
			snprintf(file,
			         255,
			         "%s",
			         token);  // 255 is the size of the destination buffer
		}
		token = strtok(NULL, "/");
	}
}

int
rootHasDirectory(const char *path)
{
	printf("[debug] fisopfs_mkdir: rootHasDirectory - path: %s\n", path);
	for (int i = 0; i < root->numSubdirectories; i++) {
		if (root->subdirectories[i] != NULL) {
			if (strcmp(root->subdirectories[i]->name, path) == 0)
				return 1;
		}
	}
	return 0;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	if (getEntries() >= MAX_ENTRIES)
		return -ENOSPC;

	const char *path_plus_one = path;
	path_plus_one++;
	if (strchr(path_plus_one, '/') == NULL) {
		// esta en root
		if (!rootHasDirectory(path_plus_one)) {
			addDir(path_plus_one, 1);
			return 0;
		} else {
			return -EEXIST;
		}
	}
	return -EPERM;
}


static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	if (strcmp(path, "/") == 0) {
		st->st_uid = getuid();
		st->st_gid = getgid();
		st->st_mode = FOLDER_ST_MODE;
		st->st_nlink = 2;
		return 0;
	}

	char folder[256];
	char file[256];
	splitPath(path, folder, file);
	// EN EL PRIMERO DEVUELVE EL PRIMER DIRECTORIO/ARCHIVO
	// EN EL SEGUNDO DEVUELVE EL ARCHIVO, EN EL CASO DE QUE EL PRIMERO SEA UN DIRECTORIO
	// EL SEGUNDO PUEDE SER NULL SI SE BUSCA UN ARCHIVO EN ROOT
	if (strlen(folder) > 0 && strlen(file) == 0) {  // 0 niveles de recursion
		st->st_uid = getuid();
		st->st_gid = getgid();
		struct Directory *dir = searchDir(folder);
		if (!dir) {  // si no encuentra el dir, asumimos que es un file en root
			struct File *f = searchFile(folder, NULL);
			if (!f)  // No existe el file
				return -ENOENT;
			st->st_mode = FILE_ST_MODE;
			st->st_nlink = 1;
			st->st_size = f->size;
			st->st_atime = f->last_access;
			st->st_mtime = f->last_modification;
			st->st_ctime = f->last_modification;
			return 0;
		}
		st->st_mode = FOLDER_ST_MODE;
		st->st_nlink = 2;
		st->st_atime = dir->last_access;
		st->st_mtime = dir->last_modification;
		st->st_ctime = dir->last_modification;


		return 0;
	}

	if (strlen(folder) > 0 && strlen(file) > 0) {  // 1 nivel de recursion

		struct Directory *dir = searchDir(folder);
		if (!dir)  // No encuentra el dir
			return -ENOENT;
		struct File *f = searchFile(file, dir);
		if (!f)  // No existe el file en ese dir
			return -ENOENT;
		st->st_uid = getuid();
		st->st_gid = getgid();
		st->st_mode = FILE_ST_MODE;
		st->st_nlink = 1;
		st->st_size = f->size;
		st->st_atime = f->last_access;
		st->st_mtime = f->last_modification;
		st->st_ctime = f->last_modification;
		return 0;
	}

	return -ENOENT;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	// Si nos preguntan por el directorio raiz
	if (strcmp(path, "/") == 0) {
		printf("[debug] readdir - root: %d files, %d dirs and %d total "
		       "entries\n",
		       root->numFiles,
		       root->numSubdirectories,
		       getEntries());

		for (int i = 0; i < root->numFiles; i++) {
			if (root->files[i] != NULL)
				filler(buffer, root->files[i]->name, NULL, 0);
		}

		for (int j = 0; j < root->numSubdirectories; j++) {
			if (root->subdirectories[j] != NULL)
				filler(buffer,
				       root->subdirectories[j]->name,
				       NULL,
				       0);
		}

		return 0;
	}

	char folder[256];
	char file[256];
	splitPath(path, folder, file);

	if (strlen(folder) > 0 && strlen(file) == 0) {
		struct Directory *dir = searchDir(folder);
		if (!dir)  // No encuentra el dir
			return -ENOENT;

		printf("[debug] readdir - dir %s: %d files\n",
		       dir->name,
		       dir->numFiles);

		for (int i = 0; i < dir->numFiles;
		     i++) {  // Llama a filler por c/file
			if (dir->files[i] != NULL)
				filler(buffer, dir->files[i]->name, NULL, 0);
		}
		return 0;
	}
	return -ENOENT;
}

static int
fisopfs_read(const char *path,
             char *buf,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	struct File *f;
	char folder[256];
	char file[256];
	splitPath(path, folder, file);

	if (strlen(folder) > 0 && strlen(file) == 0) {
		f = searchFile(folder, NULL);
	} else if (strlen(folder) > 0 && strlen(file) > 0) {
		struct Directory *dir = searchDir(folder);
		if (!dir)
			return -ENOENT;
		f = searchFile(file, dir);
	} else {
		return -ENOENT;
	}

	if (offset < 0 || offset > f->size)
		return -EINVAL;  // Invalid argument

	size_t to_read = (offset + size > f->size) ? (f->size - offset) : size;

	strncpy(buf, f->content + offset, to_read);

	struct timespec times[2];
	clock_gettime(CLOCK_REALTIME, &times[0]);  // Obtener el tiempo actual
	times[1].tv_sec =
	        f->last_modification;  // Si el archivo estaba abierto, mantener
	                               // atime, de lo contrario, establecer mtime igual a atime
	times[1].tv_nsec = 0;
	fisopfs_utimens(path, times);
	return to_read;  // Return the number of bytes read
}

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	struct File *f;
	char folder[256];
	char file[256];
	splitPath(path, folder, file);

	if (strlen(folder) > 0 && strlen(file) == 0) {
		f = searchFile(folder, NULL);
	} else if (strlen(folder) > 0 && strlen(file) > 0) {
		struct Directory *dir = searchDir(folder);
		if (!dir)
			return -ENOENT;
		f = searchFile(file, dir);
	} else {
		return -ENOENT;
	}

	if (offset < 0 || offset > f->size)
		return -EINVAL;  // Invalid argument

	size_t to_write = (offset + size > MAX_FILE_LENGHT)
	                          ? (MAX_FILE_LENGHT - offset)
	                          : size;

	memcpy(offset + f->content, buffer, to_write);

	if (offset + to_write > f->size) {
		f->size = offset + to_write;
	}

	struct timespec times[2];
	clock_gettime(CLOCK_REALTIME, &times[1]);  // Obtener el tiempo actual
	times[0].tv_sec =
	        f->last_access;  // Si el archivo estaba abierto, mantener atime,
	                         // de lo contrario, establecer mtime igual a atime
	times[0].tv_nsec = 0;
	fisopfs_utimens(path, times);

	return to_write;
}

int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[DEBUG] fisopfs_utimens - path: %s\n", path);

	// Obtener el archivo correspondiente a 'path'
	struct File *f;
	char folder[256];
	char file[256];
	splitPath(path, folder, file);

	if (strlen(folder) > 0 && strlen(file) == 0) {
		f = searchFile(folder, NULL);
	} else if (strlen(folder) > 0 && strlen(file) > 0) {
		struct Directory *dir = searchDir(folder);
		if (!dir)
			return -ENOENT;
		f = searchFile(file, dir);
	} else {
		return -ENOENT;
	}
	printf("[DEBUG] fisopfs_utimens - last_access: %ld, last_modification: "
	       "%ld\n",
	       f->last_access,
	       f->last_modification);
	// Actualizar los tiempos de acceso y modificación usando solo los segundos
	f->last_access = tv[0].tv_sec;
	f->last_modification = tv[1].tv_sec;
	printf("[DEBUG] New fisopfs_utimens - last_access: %ld, "
	       "last_modification: %ld\n",
	       f->last_access,
	       f->last_modification);
	return 0;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	// suponiendo que alguien mas checkea que no exista un archivo con el mismo nombre
	printf("[debug] fisopfs_create - path: %s\n", path);
	if (!root)
		return -ENOANO;
	if (getEntries() >= MAX_ENTRIES)
		return -ENOSPC;

	char folder[256];
	char file[256];
	splitPath(path, folder, file);

	if (strlen(folder) > 0 && strlen(file) == 0) {
		addFile(folder, &root->numFiles, &root->files);
		printf("[debug] fisopfs_create - files in root: %d\n",
		       root->numFiles);
		return 0;

	} else if (strlen(folder) > 0 && strlen(file) > 0) {
		struct Directory *dir;
		dir = searchDir(folder);
		if (!dir)
			return -ENOENT;

		addFile(file, &dir->numFiles, &dir->files);
		printf("[debug] fisopfs_create - files in folder: %d\n",
		       dir->numFiles);
		return 0;
	}
	return -EINVAL;
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);

	char folder[256];
	char file[256];
	splitPath(path, folder, file);

	if (strlen(folder) > 0 && strlen(file) == 0) {
		struct Directory *dir = searchDir(folder);
		if (!dir)
			return -ENOENT;

		// Check if the directory is empty
		if (dir->numFiles > 0)
			return -ENOTEMPTY;

		delDir(dir);

		return 0;
	}
	return -EINVAL;
}

int
delFile(struct File *f, int *num_files, struct File ***arr)
{
	printf("[debug] delFile\n");
	int indexToDelete = -1;
	for (int i = 0; i < *num_files; ++i) {
		if ((*arr)[i] == f) {
			indexToDelete = i;
			break;
		}
	}
	if (indexToDelete == -1)
		return -EAGAIN;
	free((*arr)[indexToDelete]);
	for (size_t i = indexToDelete; i < (*num_files) - 1; ++i) {
		(*arr)[i] = (*arr)[i + 1];
	}

	*arr = (struct File **) realloc(*arr,
	                                (*num_files - 1) * sizeof(struct File *));
	--(*num_files);
	return 0;
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	char folder[256];
	char file[256];
	splitPath(path, folder, file);

	if (strlen(folder) > 0 && strlen(file) > 0) {
		// Borrar archivo en un dir
		struct Directory *dir = searchDir(folder);
		if (!dir)
			return -ENOENT;

		struct File *f = searchFile(file, dir);
		if (!f)
			return -ENOENT;

		return delFile(f, &dir->numFiles, &dir->files);

		return 0;
	} else if (strlen(folder) > 0 && strlen(file) == 0) {
		// Borrar archivo en root
		struct File *f = searchFile(folder, NULL);
		if (!f)
			return -ENOENT;

		return delFile(f, &root->numFiles, &root->files);
	}

	return -ENOENT;
}

static int
fisopfs_statfs(const char *path, struct statvfs *stbuf)
{
	stbuf->f_namemax = MAX_NAME_LENGTH - 1;
	printf("statfs - path: %s, namemax: %ld\n",
	       path,
	       (unsigned long int) stbuf->f_namemax);

	return 0;
}

static int
fisopfs_rename(const char *oldpath, const char *newpath)
{
	printf("[debug] fisopfs_rename - oldpath: %s, newpath: %s\n",
	       oldpath,
	       newpath);
	char folder[256];
	char file[256];
	char folder2[256];
	char file2[256];
	splitPath(oldpath, folder, file);
	splitPath(newpath, folder2, file2);

	if (strlen(folder) > 0 && strlen(file) == 0) {
		// en root
		struct File *f = searchFile(folder, NULL);
		if (f) {
			replaceName(f->name, folder2);
			return 0;

		} else {
			struct Directory *d = searchDir(folder);
			if (!d)
				return -ENOENT;
			replaceName(d->name, folder2);
			return 0;
		}

	} else if (strlen(folder) > 0 && strlen(file) > 0) {
		struct Directory *dir;
		dir = searchDir(folder);
		if (!dir)
			return -ENOENT;
		struct File *f;
		f = searchFile(file, dir);
		if (!f)
			return -ENOENT;
		replaceName(f->name, file2);
		return 0;
	}
	return -EINVAL;
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.write = fisopfs_write,
	.mkdir = fisopfs_mkdir,
	.utimens = fisopfs_utimens,
	.create = fisopfs_create,
	.init = fisopfs_init,
	.destroy = fisopfs_destroy,
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
	.rename = fisopfs_rename,
	.statfs = fisopfs_statfs,
};

int
main(int argc, char *argv[])
{
	if (argc != 3) {
		return -EINVAL;
	}
	strncpy(filename, argv[2], MAX_NAME_LENGTH - 1);
	filename[MAX_NAME_LENGTH - 1] = '\0';
	strncat(filename, ".fisopfs", 8);
	return fuse_main(argc, argv, &operations, NULL);
}
