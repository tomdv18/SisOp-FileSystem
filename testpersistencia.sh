cd prueba
echo ""
echo -e "\033[1;36mContinuacion Test 9\033[0m" 
echo -e "\033[1;36mTest de persistencia de archivos y directorios\033[0m"

echo -e "\033[1mAl hacer ls debería haber un archivo y 4 directorios\033[0m"
ls
echo ""
echo -e "\033[1mDentro del archivo debe decir Hola!\033[0m"
echo -e "\033[1mLe hacemos un cat al archivo\033[0m"
cat archivo_ejemplo
echo ""
echo "-----------------------------------"
echo ""
echo -e "\033[1mEl primer directorio debería estar vacío\033[0m"
echo -e "\033[1mEntrando al primer directorio, hago ls\033[0m"
cd dir_1
ls -a
echo ""
echo "-----------------------------------"
echo ""
cd ..
echo -e "\033[1mEl segundo directorio debería contener un archivo, le hacemos ls\033[0m"
cd dir_2
ls
echo -e "\033[1mEl archivo debe contener la siguiente frase: Soy el archivo escondido 1\033[0m"
echo -e "\033[1mHacemos un cat del archivo\033[0m"
cat prueba_persistencia

echo ""
echo "-----------------------------------"
echo ""
cd .. 
echo -e "\033[1mEl tercer directorio debería estar vacío\033[0m"
echo -e "\033[1mEntrando al tercer directorio, hago ls\033[0m"
cd dir_3
ls -a

echo ""
echo "-----------------------------------"
echo ""
cd ..
echo -e "\033[1mEl cuarto directorio debería contener un archivo, le hacemos ls\033[0m"
cd dir_4
ls
echo -e "\033[1mEl archivo debe contener la siguiente frase: Soy el archivo escondido 2\033[0m"
echo -e "\033[1mHacemos un cat del archivo\033[0m"
cat prueba_persistencia_2
