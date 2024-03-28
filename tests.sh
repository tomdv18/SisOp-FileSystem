cd prueba
echo ""
echo -e "\033[1;36mTest 1\033[0m"
echo -e "\033[1;36mCreo un archivo\033[0m"
echo ""
echo "Hola" > archivo.txt
echo -e "\033[1mAl hacer ls, solo debería estar archivo\033[0m"
echo -e "\033[1mls\033[0m"
ls

echo "-----------------------------------"


echo -e "\033[1;36mTest 2\033[0m"
echo -e "\033[1;36mLeo contenido del archivo creado, debería decir Hola\033[0m"
echo ""
cat archivo.txt
echo -e "\033[1mModifico el contenido del archivo, agregando contenido al final\033[0m"
echo "Contenido agregado" >> archivo.txt
cat archivo.txt

echo "-----------------------------------"


echo -e "\033[1;36mTest 3\033[0m"
echo -e "\033[1;36mCambio el contenido del archivo completamente\033[0m"
echo ""
echo -e "\033[1mContenido anterior:\033[0m"
cat archivo.txt
echo -e "\033[1mModifico el contenido del archivo\033[0m"
printf "Este es el nuevo contenido \n" > archivo.txt
cat archivo.txt

echo "-----------------------------------"


echo -e "\033[1;36mTest 4\033[0m"
echo -e "\033[1;36mCreo y elimino un nuevo archivo\033[0m"
echo ""
echo -e "\033[1mCreo el archivo nuevo y hago ls\033[0m"
echo "Archivo nuevo" > archivo_nuevo.txt
ls
echo -e "\033[1mElimino un archivo y hago ls\033[0m"
rm archivo_nuevo.txt
ls

echo "-----------------------------------"

echo -e "\033[1;36mTest 5\033[0m"
echo -e "\033[1;36mPrueba de stats\033[0m"
echo ""
echo -e "\033[1mCreo dos archivos, uno contiene las stats del otro\033[0m"
echo "Archivo con stats" > archivo_stats
echo -e "\033[1mCreado archivo\033[0m"
ls
stat archivo_stats >> archivo_con_stats
echo -e "\033[1mLe cambio el contenido y vuelvo a pedir las stats\033[0m"
echo ""
echo "" >> archivo_con_stats
sleep 1 # Hago tiempo para que se note en las stats
echo "Texto nuevo nuevo nuevo" > archivo_stats
stat archivo_stats >> archivo_con_stats
echo -e "\033[1mSe deben ver las stats de antes y de después de los cambios\033[0m"
cat archivo_con_stats

rm archivo_stats
rm archivo_con_stats


echo "-----------------------------------"

echo -e "\033[1;36mTest 6\033[0m"
echo -e "\033[1;36mCreo un nuevo directorio\033[0m"
echo ""
echo -e "\033[1mCreo el directorio nuevo y hago ls\033[0m"
mkdir nuevo_directorio
ls

echo "-----------------------------------"


echo -e "\033[1;36mTest 7\033[0m"
echo -e "\033[1;36mAccedo al nuevo directorio\033[0m"
echo ""
echo -e "\033[1mls\033[0m"
ls
echo -e "\033[1mEntro al directorio y hago ls\033[0m"
cd nuevo_directorio
ls -a

echo "-----------------------------------"


echo -e "\033[1;36mTest 8\033[0m"
echo -e "\033[1;36mLos directorios tienen stats\033[0m"
echo ""
cd ..
echo -e "\033[1mls\033[0m"
ls
echo -e "\033[1mstat\033[0m"
stat nuevo_directorio

echo "-----------------------------------"

echo -e "\033[1;36mInicio Test 9\033[0m"
echo -e "\033[1;36mTest de persistencia de archivos y directorios\033[0m"
rm archivo.txt
rm -r nuevo_directorio
echo ""
echo -e "\033[1mCreo 4 directorios más\033[0m"
mkdir dir_1
mkdir dir_2
mkdir dir_3
mkdir dir_4
echo "Hola!" > archivo_ejemplo
echo -e "\033[1mEstado de la carpeta:\033[0m"
ls
echo -e "\033[1mPongo un archivo dentro del segundo y del cuarto\033[0m"
cd dir_2
echo "Soy el archivo escondido 1" >prueba_persistencia

cd ..
cd dir_4
echo "Soy el archivo escondido 2 " >prueba_persistencia_2

echo "-----------------------------------"
cd ..

cd dir_2
echo -e "\033[1mContenido directorio 2:\033[0m"
ls

cd ..
cd dir_4
echo -e "\033[1mContenido directorio 4:\033[0m"
ls
exit
