#!/bin/bash

# Script para ejecutar módulo por 10 segundos con limpieza completa
# Uso: ./ejecutar_modulo.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODULE_NAME="module_thread_2"

echo "=== EJECUTOR DE MÓDULO POR 10 SEGUNDOS ==="
echo

# Paso 1: Limpieza de módulos existentes
echo "1. Limpiando módulos existentes..."
sudo rmmod utn-fra-td3-tp5 2>/dev/null || true
sudo rmmod $MODULE_NAME 2>/dev/null || true
sudo rmmod hilo_simple 2>/dev/null || true
echo "   ✓ Módulos limpiados"

# Paso 2: Limpiar compilación anterior
echo "2. Limpiando compilación anterior..."
make clean > /dev/null 2>&1 || true
echo "   ✓ Compilación limpiada"

# Paso 3: Compilar módulo
echo "3. Compilando módulo..."
if make all > /dev/null 2>&1; then
    echo "   ✓ Módulo compilado"
else
    echo "   ❌ Error en compilación"
    exit 1
fi

# Paso 4: Limpiar buffer dmesg
echo "4. Limpiando buffer del kernel..."
sudo dmesg -c > /dev/null 2>&1
echo "   ✓ Buffer limpiado"

# Paso 5: Cargar módulo
echo "5. Cargando módulo..."
if sudo insmod build/$MODULE_NAME.ko; then
    echo "   ✓ Módulo cargado"
else
    echo "   ❌ Error cargando módulo"
    exit 1
fi

# Paso 6: Esperar 10 segundos mostrando mensajes
echo "6. Ejecutando por 10 segundos..."
echo "   --- INICIO MENSAJES DEL KERNEL ---"
(timeout 10s dmesg -w) &
DMESG_PID=$!

# Contador regresivo
for i in {10..1}; do
    echo -ne "   Tiempo restante: ${i}s\r"
    sleep 1
done
echo -e "\n"

# Matar el proceso dmesg
kill $DMESG_PID 2>/dev/null || true

# Paso 7: Descargar módulo
echo "7. Descargando módulo..."
if sudo rmmod $MODULE_NAME; then
    echo "   ✓ Módulo descargado"
else
    echo "   ❌ Error descargando módulo"
    exit 1
fi

# Paso 8: Mostrar mensajes finales
echo "8. Mensajes finales del kernel:"
sudo dmesg | tail -10

# Paso 9: Limpieza final
echo "9. Limpieza final..."
make clean > /dev/null 2>&1
echo "   ✓ Limpieza completada"

echo
echo "=== EJECUCIÓN FINALIZADA ==="