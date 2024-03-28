# fisop-fs

Ejecucion: ./fisopfs -f prueba
Ejecucion de pruebas: ./init.sh en una consola y ./tests.sh en otra

Las 3 estructuras del FS son: Master, File y Directory.

Master es el root, y es la unica carpeta capaz de contener tanto Files como Directorys.
Directory es una carpeta, contiene un nombre, Files y fechas de modificacion y acceso.
File es un archivo que contiene datos, un nombre y fechas de modificacion y acceso.

Hay una variable global que es un puntero a root
Tambien hay una variable global que es el nombre del archivo de load/save

La funcion principal de parseo es splitPath, recibe un path y 2 punteros a char. El path puede ser: /file, /dir o /dir/file.
En los dos primeros casos deja el nombre del file/dir en el primer puntero, y deja el string vacio en el segundo.
En el tercer caso, deja el nombre del dir en el primer puntero y el nombre del file en el segundo.

Las funciones que se usan para localizar los files o dirs son searchFile y searchDir
searchFile, dado un nombre (sacado de splitPath) y un puntero a dir (si es NULL se toma root) devuelve el file indicado.
searchDir, dado un nombre (sacado de splitPath) encuentra el dir con ese nombre en root.

Un caso normal de uso sería:

```
char p1[256];
char p2[256];
splitPath(path, p1, p2);

if (strlen(p1) > 0 && strlen(p2) == 0) { //Es un file o dir en root
    struct Directory* dir = searchDir(p1);
    if (dir){
        //Logica de dir
        return;
    } //si no se encuentra, es un file o no existe

    struct File* f = searchFile(p1, NULL);
	if (!f)//No existe el file
		return -ENOENT;
    //Logica de File

} else if (strlen(p1) > 0 && strlen(p2) > 0) {//file dentro de un dir
    struct Directory* dir = searchDir(p1);	
    if (!dir)//No encuentra el dir
        return -ENOENT;
    struct File* f = searchFile(p2, dir);
    if (!f)//No existe el file en ese dir
        return -ENOENT;
    //Logica de File
}
```

Persistencia:
El orden de guardado es: 
    - Numero de archivos en root
        - Los N archivos
    - Numero de dirs en root
        (dentro de saveDir, por c/dir)
        - Nombre del dir
        - Numero de archivos del dir
        - Fechas del dir
            - Los N archivos

El orden de carga es el mismo:
    - Numero de archivos en root
        - Los N archivos (llama a addFile por c/archivo, despues con fread sobreescribe todos los campos)
    - Numero de dirs en root (crea los archivos con addDir, y sobreescribe sus campos con loadDir)
        (dentro de loadDir, por c/dir)
        - Nombre del dir
        - Numero de archivos del dir
        - Fechas del dir
            - Los N archivos

Las funciones usadas para guardar son:
    .destroy -> saveDir (por c/dir), los files se escriben directamente
Las funciones usadas para cargar son:
    .init -> addDir y loadDir (por c/dir), addFile(por c/file, en root o en dir)

Funciones internas (no fisopfs_):

getEntries(): devuelve la cantidad de carpetas y archivos en todo el sistema

createDir(const char*, int): devuelve un puntero a un directorio creado en memoria dinamica, si un dir no es nuevo (se esta cargando de un file) no    carga los tiempos de modificacion/acceso

addDir(const char*, int): aumenta el tamaño de root->subdirectories en 1 y agrega un dir, llama a createDir

delDir(struct Directory*): borra el directorio de root->subdirectories y decrementa su tamaño en 1

replaceName(char*, const char*): cambia un string por otro, usado en rename

createFile(const char*): devuelve un puntero a un File creado en memoria dinamica

saveDir(FILE*, struct Directory*): guarda el directorio indicado en el archivo

loadDir(struct Directory*, FILE*): carga los datos en el directorio indicado desde el archivo

searchFile(const char*, struct Directory*): devuelve un puntero a un file, busca el file en el dir, si el dir es NULL, busca en root

searchDir(const char*): devuelve un puntero al dir especificado

rootHasDirectory(const char*): devuelve 1 si hay un dir con el nombre especificado, 0 si no

addFile(const char*, int*, struct File***): recibe una referencia a un array de punteros a File, incrementa el tamaño del array en 1, le agrega un  archivo con el nombre especificado e incrementa el puntero a int en 1. La implementacion difiere tanto de addDir ya que se le pueden agregar archivos tanto a struct Master como a struct Directory, por lo que, para no hacer 2 funciones iguales, esta funcion recibe referencias a numFiles y al array files

delFile(struct File*, int*, struct File***): recibe una referencia a un array de punteros a File, decrementa el tamaño del array en 1, le borra el archivo especificado y decrementa el puntero a int en 1. Implementado igual a addFile por las mismas razones.