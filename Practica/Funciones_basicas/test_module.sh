#!/bin/bash

# =============================================
# Script: test_module.sh
# Descripción: Automatiza prueba completa de módulo kernel
# Uso: ./test_module.sh
# =============================================

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Función para imprimir mensajes
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Verificar que estamos en el directorio correcto
if [ ! -f "hello_kernel.c" ]; then
    print_error "No se encuentra hello_kernel.c en el directorio actual"
    echo "Ejecuta el script desde el directorio de tu módulo"
    exit 1
fi

print_status "=== INICIANDO PRUEBA AUTOMÁTICA DEL MÓDULO KERNEL ==="

# Paso 1: Limpiar compilaciones anteriores
print_status "1. Limpiando compilaciones anteriores..."
make clean
if [ $? -eq 0 ]; then
    print_success "Limpieza completada"
else
    print_warning "Limpieza mostró advertencias (puede ser normal si no hay build previo)"
fi

# Paso 2: Compilar módulo
print_status "2. Compilando módulo..."
make all
if [ $? -eq 0 ]; then
    print_success "Compilación exitosa"
else
    print_error "Compilación falló"
    exit 1
fi

# Paso 3: Verificar que el módulo se creó
if [ ! -f "build/hello_kernel.ko" ]; then
    print_error "No se encontró hello_kernel.ko en build/"
    exit 1
fi
print_success "Módulo compilado: build/hello_kernel.ko"

# Paso 4: Limpiar buffer del kernel
print_status "3. Limpiando buffer del kernel..."
sudo dmesg -C
if [ $? -eq 0 ]; then
    print_success "Buffer del kernel limpiado"
else
    print_error "No se pudo limpiar el buffer del kernel"
    exit 1
fi

# Paso 5: Cargar módulo
print_status "4. Cargando módulo en el kernel..."
make insmod
if [ $? -eq 0 ]; then
    print_success "Módulo cargado exitosamente"
else
    print_error "Fallo al cargar el módulo"
    exit 1
fi

# Paso 6: Verificar que el módulo está cargado
print_status "5. Verificando módulo cargado..."
if lsmod | grep -q "hello_kernel"; then
    print_success "Módulo confirmado en lsmod"
else
    print_error "Módulo no aparece en lsmod"
    exit 1
fi

# Paso 7: Esperar y mostrar mensajes de carga
print_status "6. Esperando 2 segundos..."
sleep 2

print_status "7. Mensajes de carga del módulo:"
echo "----------------------------------------"
make dmesg
echo "----------------------------------------"

# Paso 8: Esperar antes de descargar
print_status "8. Esperando 3 segundos antes de descargar..."
sleep 3

# Paso 9: Descargar módulo
print_status "9. Descargando módulo..."
make rmmod
if [ $? -eq 0 ]; then
    print_success "Módulo descargado exitosamente"
else
    print_error "Fallo al descargar el módulo"
    exit 1
fi

# Paso 10: Verificar que el módulo se descargó
print_status "10. Verificando módulo descargado..."
if ! lsmod | grep -q "hello_kernel"; then
    print_success "Módulo confirmado como descargado"
else
    print_error "Módulo aún aparece en lsmod"
    exit 1
fi

# Paso 11: Mostrar mensajes completos
print_status "11. Mensajes completos del kernel:"
echo "----------------------------------------"
make dmesg
echo "----------------------------------------"

print_success "=== PRUEBA COMPLETADA EXITOSAMENTE ==="
echo ""
print_status "Resumen:"
echo "  - Compilación: ✅"
echo "  - Carga: ✅" 
echo "  - Ejecución: ✅"
echo "  - Descarga: ✅"
echo "  - Mensajes: ✅"
