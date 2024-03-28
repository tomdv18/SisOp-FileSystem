# fisop-fs

Ejecucion: ./fisopfs -f prueba

Las 3 estructuras del FS son: Master, File y Directory.

Master es el root, y es la unica carpeta capaz de contener tanto Files como Directorys.
Directory es una carpeta, contiene un nombre y Files.
File es un archivo que contiene datos, un nombre y TODO fecha de modificacion y creacion.

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
        - Nombre del dir
        - Numero de archivos del dir
        - Los N archivos

Las funciones usadas para guardar son: .destroy -> saveDir (por c/dir)
Las funciones usadas para cargar son: .init -> loadDir (por c/dir), addFile(por c/file, en root o en dir)