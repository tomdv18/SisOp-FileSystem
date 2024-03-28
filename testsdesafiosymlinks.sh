cd prueba
echo ""
echo -e "\033[1;36mTest 10\033[0m" 
echo -e "\033[1;36mTest de symlinks\033[0m"




ls -al
echo -e "\033[1mSe crea un symlink\033[0m"
ln -s dir_2/ link
ls -al

echo -e "\033[1mSe puede eliminar un symlink\033[0m"
rm link

ls -al
echo "-----------------------------------"


echo -e "\033[1;36mTest 11\033[0m" 
echo -e "\033[1;36mTest de symlinks\033[0m"

echo -e "\033[1mSe crean dos symlinks, pueden apuntar al mismo lugar\033[0m"

ln -s dir_2/ link
echo -e "\033[1mSymlink creado, intento crear otro\033[0m"
ln -s dir_2/ link2
ls -al
rm link
rm link2
echo "-----------------------------------"


echo -e "\033[1;36mTest 12\033[0m" 
echo -e "\033[1;36mTest de symlinks\033[0m"

echo -e "\033[1mAl hacer cd dentro de un symlink a dir_2, encuentro el archivo dentro del directorio\033[0m"

ln -s dir_2/ link
ls -al
echo -e "\033[1mcd link \033[0m"
cd link
echo -e "\033[1mls \033[0m"
ls
cd ..
rm link

echo "-----------------------------------"
