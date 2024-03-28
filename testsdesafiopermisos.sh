cd prueba
echo ""
echo -e "\033[1;31mIMPORTANTE: TIENE QUE HABER UN USUARIO LLAMADO PRUEBA\033[0m" 
echo -e "\033[1;31msudo adduser prueba\033[0m" 
sudo adduser prueba

echo -e "\033[1;36mTest 13\033[0m" 
echo -e "\033[1;36mTest de permisos\033[0m"
echo -e "\033[1mCreo un archivo, tiene todos los permisos y el usuario por defecto\033[0m"

echo "archivo nuevo" > permisos.txt
echo -e "\033[1mArchivo creado, veo sus permisos\033[0m"
stat permisos.txt
echo "-----------------------------------"

echo -e "\033[1;36mTest 14\033[0m" 
echo -e "\033[1;36mTest de permisos\033[0m"
echo -e "\033[1mCreo un directorio, tiene todos los permisos y el usuario por defecto\033[0m"

mkdir direc_permisos
echo -e "\033[1mDirectorio creado, veo sus permisos\033[0m"
stat direc_permisos
echo "-----------------------------------"

echo -e "\033[1;36mTest 15\033[0m" 
echo -e "\033[1;36mTest de permisos\033[0m"
echo -e "\033[1mVeo sus permisos previos\033[0m"
ls -al permisos.txt
echo -e "\033[1mAl archivo creado, le puedo cambiar sus permisos\033[0m"
echo -e "\033[1mchmod 750 permisos.txt \033[0m"
chmod 750 permisos.txt
echo -e "\033[1mVeo que sus permisos cambian\033[0m"
ls -al permisos.txt
echo "-----------------------------------"


echo -e "\033[1;36mTest 16\033[0m" 
echo -e "\033[1;36mTest de permisos\033[0m"
echo -e "\033[1mSe puede cambiar el duenio del archivo \033[0m"
chmod 777 permisos.txt
echo -e "\033[1mchown prueba:prueba permisos.txt \033[0m"
chown prueba:prueba permisos.txt
echo -e "\033[1mVeo que su owner cambia\033[0m"
ls -al permisos.txt
echo "-----------------------------------"

echo -e "\033[1;36mTest 17\033[0m" 
echo -e "\033[1;36mTest de permisos\033[0m"
echo -e "\033[1mPierdo el acceso si no tengo permisos \033[0m"
echo -e "\033[1mchmod 000 permisos.txt \033[0m"
chmod 000 permisos.txt
ls -al permisos.txt
echo -e "\033[1mTrato de hacer cat permisos.txt\033[0m"
cat permisos.txt
echo "-----------------------------------"
