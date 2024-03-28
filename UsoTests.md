Estos archivos .sh son para correr test de forma semiautomatica, comparando los outputs de los distintos tests. Se tienen que tener dos terminales abiertas (Al estilo de lo que se hace con docker attach).
En la terminal base se debe poner  ./init.sh
En el terminal adjunta se corre ./test.sh

Para probar la persistencia:

Se debe terminar la ejecucion de la terminal base con ctrl + c

En esa misma terminal se debe correr ./persistencia.sh

En la otra terminal, se ejecuta ./testpersistencia.sh
