import os
import pandas as pd

# Listar todos los archivos que comienzan con "output_" y terminan en ".txt"
result_files = [f for f in os.listdir('.') if f.startswith('output_') and f.endswith('.txt')]

# Crear una lista para almacenar los datos procesados
data = []

# Procesar cada archivo
for file in result_files:
    try:
        # Estructura esperada:
        # 0    1    2    3      4        5    6     7
        # "output", n,   m,   n_gen,  tam_pob, np,  ngm,  T{nthreads}.txt
        parts = file.split('_')
        # parts[0] = "output"
        # parts[1] = n        (int)
        # parts[2] = m        (int)
        # parts[3] = n_gen    (int)
        # parts[4] = tam_pob  (int)
        # parts[5] = np       (int)
        # parts[6] = ngm      (int)
        # parts[7] = "T8.txt" (ejemplo)

        # 1) Convertir partes iniciales
        n = int(parts[1])
        m = int(parts[2])
        n_gen = int(parts[3])
        tam_pob = int(parts[4])
        np_val = int(parts[5])
        ngm = int(parts[6])  # "5"

        # 2) Extraer nthreads a partir de "T8.txt"
        last_part = parts[7]             # "T8.txt"
        # Eliminar extensión .txt
        last_part_no_ext = last_part.split('.')[0]  # "T8"
        # Quitar la 'T'
        if last_part_no_ext.startswith('T'):
            nthreads_str = last_part_no_ext[1:]  # "8"
        else:
            raise ValueError(f"No se encontró 'T' en la última parte del nombre: {file}")
        nthreads = int(nthreads_str)  # 8

        # Leer el contenido del archivo
        with open(file, 'r') as f:
            lines = f.readlines()
            exe_time = None
            fitness = None
            for line in lines:
                if "Execution Time" in line:
                    # Ejemplo: "Execution Time: 123.45 seg"
                    exe_time = float(line.split(":")[1].strip().split()[0])
                if "Distance" in line:
                    # Ejemplo: "Distance: 4567.89"
                    fitness = float(line.split(":")[1].strip())

            # Añadir los datos a la lista si ambos valores se encontraron
            if exe_time is not None and fitness is not None:
                data.append({
                    'n': n,
                    'm': m,
                    'n_gen': n_gen,
                    'tam_pob': tam_pob,
                    'fitness': fitness,
                    'exe_time': exe_time,
                    'np': np_val,
                    'ngm': ngm,
                    'nthreads': nthreads
                })
            else:
                print(f"Datos incompletos en el archivo: {file}")

    except Exception as e:
        print(f"Error procesando el archivo {file}: {e}")

# Convertir los datos a un DataFrame
df = pd.DataFrame(data)

# Verificar si el DataFrame tiene datos antes de procesarlo
if not df.empty:
    # Reordenar columnas: nthreads en lugar de threads
    df = df[["n", "m", "n_gen", "tam_pob", "fitness", "exe_time", "np", "ngm", "nthreads"]]

    # Exportar a Excel
    output_file = "resultados.xlsx"
    df.to_excel(output_file, index=False)
    print(f"Datos exportados a '{output_file}'")
else:
    print("No se encontraron datos válidos para exportar.")
