# ==============================================================================
# Script de experimentacion completa para Bin Packing
# Version mejorada con codificacion UTF-8
# Ejecutar desde: scripts/
# ==============================================================================

# Forzar UTF-8 para el output
$PSDefaultParameterValues['Out-File:Encoding'] = 'utf8'
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

Write-Host ""
Write-Host "===============================================" -ForegroundColor Cyan
Write-Host "  Bin Packing - Experimento Automatico" -ForegroundColor Cyan
Write-Host "===============================================" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# Configuracion (rutas desde scripts/)
# ==============================================================================
$outputDir = "..\output"
$logsDir = "$outputDir\logs"
$instancesDir = "..\instances"
$binPackingExe = "..\build\Release\bin_packing.exe"
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

# Verificar que el ejecutable existe
if (-not (Test-Path $binPackingExe)) {
    Write-Host "ERROR: No se encuentra bin_packing.exe" -ForegroundColor Red
    Write-Host "Ruta esperada: $binPackingExe" -ForegroundColor Red
    Write-Host ""
    Write-Host "Por favor, compila el proyecto primero:" -ForegroundColor Yellow
    Write-Host "  cd ..\build" -ForegroundColor Yellow
    Write-Host "  cmake --build . --config Release" -ForegroundColor Yellow
    Write-Host ""
    pause
    exit 1
}

# Crear directorios
New-Item -ItemType Directory -Force -Path $logsDir | Out-Null

# Archivos de salida
$masterLog = "$logsDir\experiment_$timestamp.txt"
$csvResults = "$outputDir\results_$timestamp.csv"

# ==============================================================================
# Inicializar archivos
# ==============================================================================
"Experimento iniciado: $(Get-Date)" | Out-File $masterLog -Encoding utf8
"" | Out-File $masterLog -Append -Encoding utf8

# CSV con resultados
$csvHeader = "Instancia,Items,Capacidad,FF,BF,WF,FFD,Gurobi,Optimo,Gap_FFD,Tiempo_seg,Utilizacion"
$csvHeader | Out-File $csvResults -Encoding utf8

Write-Host "Configuracion:" -ForegroundColor Yellow
Write-Host "  Ejecutable:   $binPackingExe"
Write-Host "  Output dir:   $outputDir"
Write-Host "  Logs dir:     $logsDir"
Write-Host "  Results:      $csvResults"
Write-Host ""

# ==============================================================================
# Funcion para parsear resultados
# ==============================================================================
function Parse-Results {
    param (
        [string]$LogContent,
        [string]$InstanceName
    )
    
    # Parsear numero de items y capacidad
    if ($LogContent -match "Items:\s*(\d+),\s*Capacidad:\s*(\d+)") {
        $items = $matches[1]
        $capacity = $matches[2]
    } else {
        $items = "N/A"
        $capacity = "N/A"
    }
    
    # Parsear resultados de heuristicas
    if ($LogContent -match "First Fit:\s*(\d+)\s*bins") {
        $ff = $matches[1]
    } else { $ff = "N/A" }
    
    if ($LogContent -match "Best Fit:\s*(\d+)\s*bins") {
        $bf = $matches[1]
    } else { $bf = "N/A" }
    
    if ($LogContent -match "Worst Fit:\s*(\d+)\s*bins") {
        $wf = $matches[1]
    } else { $wf = "N/A" }
    
    if ($LogContent -match "First Fit Decreasing:\s*(\d+)\s*bins") {
        $ffd = $matches[1]
    } else { $ffd = "N/A" }
    
    # Parsear Gurobi (buscar "Bins usados:")
    if ($LogContent -match "Bins usados:\s*(\d+)") {
        $gurobi = $matches[1]
    } else { $gurobi = "N/A" }
    
    # Verificar si es optimo (flexible con encoding)
    if ($LogContent -match "ptimo.*probado|Optimo.*probado") {
        $optimal = "SI"
    } elseif ($LogContent -match "NO.*ptima|NO.*optima") {
        $optimal = "NO"
    } else {
        $optimal = "N/A"
    }
    
    # Parsear gap (mas flexible)
    if ($LogContent -match "Gap con.*?:\s*([\d.]+)%") {
        $gap = $matches[1]
    } else {
        $gap = "N/A"
    }
    
    # Parsear tiempo
    if ($LogContent -match "Con warm start:\s*([\d.]+)\s*segundo") {
        $time = $matches[1]
    } else { $time = "N/A" }
    
    # Parsear utilizacion (mas flexible)
    if ($LogContent -match "tilizaci.*?n total:\s*([\d.]+)%|Utilizacion total:\s*([\d.]+)%") {
        if ($matches[1]) {
            $util = $matches[1]
        } elseif ($matches[2]) {
            $util = $matches[2]
        }
    } else { 
        $util = "N/A" 
    }
    
    # Retornar CSV line
    return "$InstanceName,$items,$capacity,$ff,$bf,$wf,$ffd,$gurobi,$optimal,$gap,$time,$util"
}

# ==============================================================================
# Ejecutar casos predefinidos
# ==============================================================================
Write-Host "[$(Get-Date -Format 'HH:mm:ss')] Ejecutando casos predefinidos..." -ForegroundColor Green

$predefLog = "$logsDir\casos_predefinidos_$timestamp.txt"

# Ejecutar y guardar con UTF-8
$output = & $binPackingExe 2>&1
$output | Out-File $predefLog -Encoding utf8

$content = Get-Content $predefLog -Raw -Encoding utf8

# Parsear Caso1
if ($content -match "Caso1[\s\S]*?(?=Caso2|===)") {
    $caso1Content = $matches[0]
    $caso1Line = Parse-Results -LogContent $caso1Content -InstanceName "Caso1"
    $caso1Line | Out-File $csvResults -Append -Encoding utf8
}

# Parsear Caso2
if ($content -match "Caso2[\s\S]*?(?=MODO ARCHIVO|Analisis completado|$)") {
    $caso2Content = $matches[0]
    $caso2Line = Parse-Results -LogContent $caso2Content -InstanceName "Caso2"
    $caso2Line | Out-File $csvResults -Append -Encoding utf8
}

Write-Host "  OK - Casos predefinidos completados" -ForegroundColor Green
Write-Host ""

# ==============================================================================
# Procesar instancias desde archivos
# ==============================================================================
$instances = Get-ChildItem "$instancesDir\*.txt" | Sort-Object Name
$total = $instances.Count
$current = 0

if ($total -eq 0) {
    Write-Host "ADVERTENCIA: No se encontraron instancias en $instancesDir" -ForegroundColor Yellow
    Write-Host ""
} else {
    Write-Host "Procesando $total instancias desde $instancesDir" -ForegroundColor Yellow
    Write-Host ""

    foreach ($instance in $instances) {
        $current++
        $percentComplete = [math]::Round(($current / $total) * 100)
        
        # Barra de progreso
        Write-Progress -Activity "Ejecutando experimentos" -Status "Instancia $current de $total" -PercentComplete $percentComplete -CurrentOperation $instance.Name
        
        $progressText = "$current/$total - $percentComplete%"
        Write-Host "[$progressText] $($instance.Name)" -ForegroundColor Cyan
        
        # Ejecutar bin_packing con esta instancia y guardar con UTF-8
        $instanceLog = "$logsDir\$($instance.BaseName)_$timestamp.txt"
        $output = & $binPackingExe $instance.FullName 2>&1
        $output | Out-File $instanceLog -Encoding utf8
        
        # Parsear resultados
        $content = Get-Content $instanceLog -Raw -Encoding utf8
        $csvLine = Parse-Results -LogContent $content -InstanceName $instance.BaseName
        $csvLine | Out-File $csvResults -Append -Encoding utf8
        
        Write-Host "  OK - Completado" -ForegroundColor Green
    }

    Write-Progress -Activity "Ejecutando experimentos" -Completed
}

# ==============================================================================
# Resumen final
# ==============================================================================
Write-Host ""
Write-Host "===============================================" -ForegroundColor Green
Write-Host "  Experimento Completado" -ForegroundColor Green
Write-Host "===============================================" -ForegroundColor Green
Write-Host ""
Write-Host "Resultados:" -ForegroundColor Yellow
$totalInstances = $total + 2
Write-Host "  Total instancias: $totalInstances (2 predefinidas + $total archivos)"
Write-Host "  Tabla CSV:        $csvResults"
Write-Host "  Logs detallados:  $logsDir"
Write-Host "  JSONs:            $outputDir"
Write-Host ""

# Mostrar primeras lineas del CSV
Write-Host "Vista previa de resultados:" -ForegroundColor Yellow
Get-Content $csvResults -Head 6 -Encoding utf8 | ForEach-Object { Write-Host "  $_" }

Write-Host ""
Write-Host "Para ver tabla completa: notepad $csvResults" -ForegroundColor Cyan
Write-Host ""

pause