import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os

def generate_boxplot(df_subset, algorithms, title, filename, output_dir, figsize=(7, 5), rotate_x=False):
    """Función auxiliar para generar gráficas de grupos específicos manteniendo el orden."""
    data = df_subset[df_subset['Algorithm'].isin(algorithms)]
    if data.empty:
        return
    
    plt.figure(figsize=figsize)
    
    # 'order' fuerza a que Seaborn dibuje las cajas exactamente en el orden de la lista 'algorithms'
    orden_presentes = [alg for alg in algorithms if alg in data['Algorithm'].values]
    sns.boxplot(data=data, x="Algorithm", y="Fitness", order=orden_presentes)
    
    plt.title(title)
    plt.xlabel("Variante")
    plt.ylabel("Fitness")
    
    if rotate_x:
        plt.xticks(rotation=45)
        
    plt.tight_layout()
    filepath = os.path.join(output_dir, filename)
    plt.savefig(filepath)
    plt.close()
    print(f"Generado: {filepath}")

def main():
    fname = "det_results.csv"
    output_dir = "graficas"
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    if not os.path.exists(fname):
        print(f"Error: No se encuentra el archivo {fname} en esta ruta.")
        return

    df = pd.read_csv(fname)
    
    # --- 1. MAPEO DE NOMBRES (ACTUALIZADO AL CSV) ---
    df['Algorithm'] = df['Algorithm'].replace({
        'RandomSearch': 'RANDOM',
        'GreedySearch': 'GREEDY',
        'LocalSearch': 'LOCAL',
        'AGG-Uniform': 'AGG-Uni',
        'AGG-Segment': 'AGG-Seg',
        'AGE-Uniform': 'AGE-Uni',
        'AGE-Segment': 'AGE-Seg',
        'AM-All': 'AM-All',
        'AM-Rand': 'AM-Rand',
        'AM-Best': 'AM-Best'
    })

    datasets = df['Dataset'].unique()
    ks = df['K'].unique()
    
    # --- 2. LISTAS ORDENADAS PARA CADA COMPARATIVA ---
    comp_1_global = ['RANDOM', 'GREEDY', 'LOCAL', 'AGG-Uni', 'AGG-Seg', 'AGE-Uni', 'AGE-Seg', 'AM-All', 'AM-Rand', 'AM-Best']
    comp_2_evolucion = ['AGG-Uni', 'AGE-Uni', 'AGG-Seg', 'AGE-Seg']
    comp_3_gen_vs_mem = ['AGG-Uni', 'AGG-Seg', 'AGE-Uni', 'AGE-Seg', 'AM-All', 'AM-Rand', 'AM-Best']
    comp_4_memeticos = ['LOCAL','AM-All', 'AM-Rand', 'AM-Best']

    for dataset in datasets:
        for k in ks:
            subset = df[(df['Dataset'] == dataset) & (df['K'] == k)]
            
            if subset.empty:
                continue
                
            name = dataset.split('/')[-1] 
            
            # --- COMPARATIVA 1: LOS 10 ALGORITMOS A LA VEZ ---
            generate_boxplot(subset, comp_1_global,
                             f"{name} (k={k}) - Comparativa Global (10 Algs)",
                             f"{name}_k{k}_all.png", output_dir,
                             figsize=(14, 6), rotate_x=True)

            # --- COMPARATIVA 2: AGG vs AGE y CRUCES---
            # Para ver si el estacionario converge mejor que el generacional y cuál de los dos 
            # operadores de cruce es más efectivo
            generate_boxplot(subset, comp_2_evolucion,
                             f"{name} (k={k}) - Generacional vs Estacionario",
                             f"{name}_k{k}_evolution.png", output_dir)

            # --- COMPARATIVA 3: TODOS LOS AGs vs TODOS LOS MEMÉTICOS ---
            # Comparamos el base (AGG) contra los meméticos para justificar la hibridación
            # Usamos el AGG-Uni como representante del genético puro
            generate_boxplot(subset, comp_3_gen_vs_mem,
                             f"{name} (k={k}) - Genéticos Puros vs Meméticos",
                             f"{name}_k{k}_gen_vs_mem.png", output_dir,
                             figsize=(11, 6), rotate_x=True)

            # --- COMPARATIVA 4: ESTRATEGIAS MEMÉTICAS ---
            # ¿Es mejor aplicar BL a todos, a unos pocos al azar o a los mejores?
            generate_boxplot(subset, comp_4_memeticos,
                             f"{name} (k={k}) - Comparativa entre Meméticos y BL",
                             f"{name}_k{k}_zoom_mem.png", output_dir)

if __name__ == '__main__':
    main()