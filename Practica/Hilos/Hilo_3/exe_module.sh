#!/bin/bash

# Script para ejecutar módulo por 10 segundos con limpieza total
# Uso: ./ejecutar_10s.sh

echo "=================================================="
echo "🚀 EJECUTOR CONTROLADO - 10 SEGUNDOS"
echo "=================================================="
echo

# Configuración
MODULE_NAME="module_thread_3"
MODULE_PATH="./build/${MODULE_NAME}.ko"
AUTHOR="TD3"

# Paso 1: Limpieza de módulos existentes
echo "1. 🗑️  Limpiando módulos existentes..."
sudo rmmod module_thread_1 2>/dev/null || true
sudo rmmod module_thread_2hilos 2>/dev/null || true
sudo rmmod td3_hilo_A 2>/dev/null || true
sudo rmmod td3_hilo_B 2>/dev/null || true
sudo rmmod $(lsmod | grep -i "td3\|hilo\|thread" | awk '{print $1}') 2>/dev/null || true
echo "   ✅ Módulos eliminados"

# Paso 2: Limpiar compilación anterior
echo "2. 🔨 Limpiando compilación anterior..."
make clean > /dev/null 2>&1
echo "   ✅ Build limpiado"

# Paso 3: Compilar módulo
echo "3. 📦 Compilando módulo..."
if make all > /dev/null 2>&1; then
    echo "   ✅ Módulo compilado exitosamente"
else
    echo "   ❌ Error en compilación"
    exit 1
fi

# Paso 4: Limpiar buffer del kernel
echo "4. 🧹 Limpiando buffer dmesg..."
sudo dmesg -c > /dev/null 2>&1
echo "   ✅ Buffer limpiado"

# Paso 5: Cargar módulo
echo "5. ⬆️  Cargando módulo..."
if sudo insmod ${MODULE_PATH}; then
    echo "   ✅ Módulo cargado"
else
    echo "   ❌ Error cargando módulo"
    exit 1
fi

# Paso 6: Verificar que se cargó
echo "6. 🔍 Verificando carga..."
if lsmod | grep -q "${MODULE_NAME}"; then
    echo "   ✅ Módulo cargado correctamente"
else
    echo "   ❌ Módulo no aparece en lsmod"
    exit 1
fi

# Paso 7: Ejecutar por 10 segundos
echo "7. ⏱️  Ejecutando por 10 segundos..."
echo "   -----------------------------------------"
echo "   INICIO EJECUCIÓN - $(date +%H:%M:%S)"
echo "   -----------------------------------------"

# Ejecutar dmesg en background mostrando solo mensajes TD3
(dmesg -w | grep --line-buffered "${AUTHOR}") &
DMESG_PID=$!

# Contador regresivo con barra de progreso
for i in {10..1}; do
    printf "   Tiempo restante: %2d segundos [%-10s]\r" $i $(printf '#%.0s' $(seq 1 $((10 - $i))))
    sleep 1
done
echo -e "\n"

# Paso 8: Detener monitoreo
kill ${DMESG_PID} 2>/dev/null

# Paso 9: Descargar módulo
echo "8. ⬇️  Descargando módulo..."
if sudo rmmod ${MODULE_NAME}; then
    echo "   ✅ Módulo descargado"
else
    echo "   ❌ Error descargando módulo"
    exit 1
fi

# Paso 10: Mostrar resumen final
echo "9. 📊 Resumen final:"
echo "   -----------------------------------------"
sudo dmesg | grep "${AUTHOR}" | tail -10
echo "   -----------------------------------------"
echo "   FIN EJECUCIÓN - $(date +%H:%M:%S)"

# Paso 11: Limpieza final
echo "10. 🧽 Limpieza final..."
make clean > /dev/null 2>&1
echo "   ✅ Todo limpiado"

echo
echo "=================================================="
echo "✅ EJECUCIÓN COMPLETADA EXITOSAMENTE"
echo "=================================================="