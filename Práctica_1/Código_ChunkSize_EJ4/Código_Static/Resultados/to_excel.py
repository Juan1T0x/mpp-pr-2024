import os
import pandas as pd

# Listar todos los archivos en la carpeta actual con extensión .txt
result_files = [f for f in os.listdir('.') if f.startswith('out_') and f.endswith('.txt')]

# Crear una lista para almacenar los datos procesados
data = []

# Procesar cada archivo
for file in result_files:
    try:
        # Extraer n, m, n_gen, tam_pob, nthreads, chunksize del nombre del archivo
        parts = file.split('_')
        n = int(parts[1])
        m = int(parts[2])
        n_gen = int(parts[3])
        tam_pob = int(parts[4])
        nthreads = int(parts[5])
        chunksize = int(parts[6].split('.')[0])  # Eliminar la extensión del último valor

        # Leer el contenido del archivo
        with open(file, 'r') as f:
            lines = f.readlines()
            exe_time = None
            fitness = None
            for line in lines:
                if "Execution Time" in line:
                    exe_time = float(line.split(":")[1].strip().split()[0])  # Obtiene el tiempo en segundos
                if "Distance" in line:
                    fitness = float(line.split(":")[1].strip())  # Obtiene la distancia

            # Añadir los datos a la lista si ambos valores se encontraron
            if exe_time is not None and fitness is not None:
                data.append({
                    'n': n,
                    'm': m,
                    'n_gen': n_gen,
                    'tam_pob': tam_pob,
                    'fitness': fitness,
                    'exe_time': exe_time,
                    'nthreads': nthreads,
                    'chunksize': chunksize
                })
    except Exception as e:
        print(f"Error procesando el archivo {file}: {e}")

# Convertir los datos a un DataFrame
df = pd.DataFrame(data)

# Asegurarse de que las columnas estén en el orden deseado
df = df[["n", "m", "n_gen", "tam_pob", "fitness", "exe_time", "nthreads", "chunksize"]]

# Exportar a Excel
output_file = "resultados.xlsx"
df.to_excel(output_file, index=False)

print(f"Datos exportados a '{output_file}'")
