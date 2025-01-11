import os
import pandas as pd

# Listar todos los archivos que comienzan con "output_" y terminan en ".txt"
result_files = [f for f in os.listdir('.') if f.startswith('out_') and f.endswith('.txt')]

# Crear una lista para almacenar los datos procesados
data = []

# Procesar cada archivo
for file in result_files:
    try:
        # Extraer n, m, n_gen, tam_pob, n_hilos_ini, n_hilos_fit del nombre del archivo
        parts = file.split('_')
        # Estructura: output_ N  M  N_GEN  T_POB  N_HILOS_INI  N_HILOS_FIT .txt
        n = int(parts[1])
        m = int(parts[2])
        n_gen = int(parts[3])
        tam_pob = int(parts[4])
        n_hilos_ini = int(parts[5])
        n_hilos_fit = int(parts[6].split('.')[0])  # Eliminar la extensión

        # Leer el contenido del archivo
        with open(file, 'r') as f:
            lines = f.readlines()
            
            # Valores que queremos extraer del contenido
            tiempo_for_ini = None
            exe_time = None
            
            # Buscar las líneas que nos interesan
            for line in lines:
                # Ejemplo de línea: "Tiempo FOR_INI con 1 hilos: 19.667150 seg"
                if "Tiempo FOR_INI con" in line:
                    # Extraer el valor entre los dos puntos y la palabra 'seg'
                    # Suponemos que la línea tiene formato: "Tiempo FOR_INI con X hilos: Y seg"
                    parts_line = line.split(':')
                    if len(parts_line) >= 2:
                        # El valor estará antes de 'seg'
                        valor_str = parts_line[1].split('seg')[0].strip()
                        # Convertir a float
                        tiempo_for_ini = float(valor_str)
                
                # Ejemplo de línea: "Execution Time: 19.67 sec"
                if "Execution Time" in line:
                    # Extraer el tiempo
                    # Formato: "Execution Time: XX.XX sec"
                    parts_line = line.split(':')
                    if len(parts_line) >= 2:
                        valor_str = parts_line[1].replace('sec', '').strip()
                        exe_time = float(valor_str)
            
            # Añadir los datos a la lista si ambos valores se encontraron
            if tiempo_for_ini is not None and exe_time is not None:
                data.append({
                    'n': n,
                    'm': m,
                    'n_gen': n_gen,
                    'tam_pob': tam_pob,
                    'n_hilos_ini': n_hilos_ini,
                    'n_hilos_fit': n_hilos_fit,
                    'tiempo_for_ini': tiempo_for_ini,
                    'exe_time': exe_time
                })
            else:
                print(f"Datos incompletos en el archivo: {file} (no se encontró tiempo_for_ini o exe_time)")
    except Exception as e:
        print(f"Error procesando el archivo {file}: {e}")

# Convertir los datos a un DataFrame
df = pd.DataFrame(data)

# Verificar si el DataFrame tiene datos antes de procesarlo
if not df.empty:
    # Asegurar un orden lógico de columnas
    df = df[[
        "n",
        "m",
        "n_gen",
        "tam_pob",
        "n_hilos_ini",
        "n_hilos_fit",
        "tiempo_for_ini",
        "exe_time"
    ]]

    # Exportar a Excel
    output_file = "resultados.xlsx"
    df.to_excel(output_file, index=False)
    print(f"Datos exportados a '{output_file}'")
else:
    print("No se encontraron datos válidos para exportar.")
