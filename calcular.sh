#!/bin/bash

num_cursos=$(jq length archivo.json)

for ((i = 0; i < num_cursos; i++)); do
    nombre_curso=$(jq -r ".[$i].nombre_curso" archivo.json)
    echo "Curso $((i+1)): $nombre_curso"
done

echo -n "Elegir curso: "
read INDICE
INDICE=($INDICE-1)

if [[ $INDICE -ge $num_cursos ]]; then
    echo "No existe ese curso."
    exit
fi

CURSO=$(jq -r ".[$INDICE].nombre_curso" archivo.json)
valores_pesos=($(jq -r ".[$INDICE].valores | @sh" archivo.json))
NOTA_ACTUAL=0

echo "Los valores de los pesos del curso $CURSO son:"
echo "${valores_pesos[@]}"

NOTAS=()

for ((i=0; i<6; i++)); do
    read -p "Ingrese nota $(($i+1)):" NOTA
    NOTAS="$NOTAS "$NOTA
done

# echo ${NOTAS[@]} ${valores_pesos[@]}

./promedio ${NOTAS[@]} ${valores_pesos[@]}
