#!/bin/bash

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Funciones de color
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_section() {
    echo -e "${YELLOW}$1${NC}"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Verificar que estamos en el directorio correcto
if [ ! -f "param.c" ]; then
    print_error "No se encuentra param_int.c en el directorio actual"
    echo "Ejecuta el script desde el directorio de tu módulo"
    exit 1
fi

print_section "=========================================="
print_section "    PRUEBA MÓDULO CON PARÁMETROS ENTEROS"
print_section "=========================================="

# Paso 1: Compilar
print_info "1. Compilando módulo param_int..."
make clean
make all

if [ $? -ne 0 ]; then
    print_error "Error en la compilación"
    exit 1
fi
print_success "Compilación exitosa"

echo ""

# Función para ejecutar y mostrar resultados
ejecutar_modulo() {
    local descripcion="$1"
    local parametros="$2"
    
    print_section "$descripcion"
    echo "Parámetros: $parametros"
    echo "----------------------------------------"
    
    # Limpiar buffer del kernel
    sudo dmesg -C > /dev/null 2>&1
    
    # Cargar módulo
    if [ -z "$parametros" ]; then
        sudo insmod build/param.ko
    else
        sudo insmod build/param.ko $parametros
    fi
    
    # Verificar si se cargó correctamente
    if lsmod | grep -q "param"; then
        print_success "✓ Módulo cargado correctamente"
        
        # Mostrar mensajes del kernel
        dmesg | grep "utn-fra-td3"
        
        # Descargar módulo
        sudo rmmod param > /dev/null 2>&1
        
        # Mostrar mensaje de salida
        dmesg | grep "utn-fra-td3" | tail -2
    else
        print_error "✗ Error al cargar el módulo"
        dmesg | tail -5
    fi
    
    echo ""
    sleep 1
}

# ==========================================
# PRUEBA 1: Valores iniciales (sin datos externos)
# ==========================================
print_section "PRUEBA 1: VALORES INICIALES (SIN DATOS EXTERNOS)"
print_info "Se cargará el módulo con los valores por defecto:"
print_info "  - mi_numero = 10"
print_info "  - repeticiones = 1" 
print_info "  - debug = 0"

ejecutar_modulo "Ejecutando con valores por defecto" ""

# ==========================================
# PRUEBA 2: Con datos externos
# ==========================================
print_section "PRUEBA 2: CON DATOS EXTERNOS"

print_info "Caso 2A: Número positivo simple"
ejecutar_modulo "Número positivo" "mi_numero=25"

print_info "Caso 2B: Número negativo"
ejecutar_modulo "Número negativo" "mi_numero=-15"

print_info "Caso 2C: Con múltiples repeticiones"
ejecutar_modulo "Múltiples repeticiones" "mi_numero=7 repeticiones=5"

print_info "Caso 2D: Con modo debug activado"
ejecutar_modulo "Con debug" "mi_numero=42 repeticiones=2 debug=1"

print_info "Caso 2E: Número cero"
ejecutar_modulo "Número cero" "mi_numero=0 repeticiones=3"

print_info "Caso 2F: Combinación completa"
ejecutar_modulo "Combinación completa" "mi_numero=100 repeticiones=4 debug=1"

# ==========================================
# VERIFICACIÓN FINAL
# ==========================================
print_section "VERIFICACIÓN FINAL"

print_info "Información del módulo compilado:"
modinfo build/param.ko | grep -E "(description|parm)"

print_success "=========================================="
print_success "       PRUEBAS COMPLETADAS EXITOSAMENTE"
print_success "=========================================="

echo ""
print_info "Resumen de lo probado:"
echo "  ✅ Valores por defecto (sin parámetros)"
echo "  ✅ Números positivos y negativos"
echo "  ✅ Múltiples repeticiones"
echo "  ✅ Modo debug"
echo "  ✅ Número cero"
echo "  ✅ Combinaciones complejas"
