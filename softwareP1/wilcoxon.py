import pandas as pd
from scipy.stats import wilcoxon

# 1. Cargar los datos
df = pd.read_csv('det_results.csv')

# 2. Filtrar solo Búsqueda Aleatoria y Búsqueda Local
df_filtered = df[df['Algorithm'].isin(['RandomSearch', 'LocalSearch'])]

print("RESULTADOS DEL TEST DE WILCOXON (RandomSearch vs LocalSearch)\n" + "-"*60)

# 3. Agrupar por Dataset y k para hacer el test en cada escenario
escenarios = df_filtered.groupby(['Dataset', 'K'])

for (dataset, k), group in escenarios:
    # Extraer solo las columnas de semilla y fitness
    random_df = group[group['Algorithm'] == 'RandomSearch'][['Seed', 'Fitness']]
    local_df = group[group['Algorithm'] == 'LocalSearch'][['Seed', 'Fitness']]
    
    # 4. Unir por semilla (Inner Join) para asegurar que solo usamos pares idénticos
    merged = pd.merge(random_df, local_df, on='Seed', suffixes=('_random', '_local'))
    
    # Avisar si faltan datos
    if len(random_df) != len(local_df):
        print(f"Aviso en {dataset} (k={k}): Hay {len(random_df)} datos Random y {len(local_df)} Local.")
        print(f"Se ha corregido utilizando solo las {len(merged)} semillas coincidentes.\n")
        
    if len(merged) == 0:
        continue # Si no hay coincidencias, saltar

    # 5. Aplicar el test de Wilcoxon a las columnas ya emparejadas
    stat, p_value = wilcoxon(merged['Fitness_random'], merged['Fitness_local'], alternative='greater')
    
    # Mostrar resultados
    print(f"Dataset: {dataset:15} | k: {k} | p-valor: {p_value:.3e}")
    if p_value < 0.05:
        print("Diferencia significativa (LocalSearch es estadísticamente mejor)\n")
    else:
        print("No hay diferencia significativa\n")