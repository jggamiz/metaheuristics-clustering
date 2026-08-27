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
    plt.xlabel("Algoritmo")
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
    
    # Limpiamos espacios por si acaso
    df['Algorithm'] = df['Algorithm'].astype(str).str.strip()
    
    # --- 1. MAPEO DE NOMBRES (ACTUALIZADO AL CSV DE TRAYECTORIAS) ---
    df['Algorithm'] = df['Algorithm'].replace({
        'RandomSearch': 'Random',
        'GreedySearch': 'Greedy',
        'LocalSearch': 'Local',
        'SimulatedAnnealing': 'SA',
        'SimulatedAnnealingFast': 'SA-Fast',
        'BMB': 'BMB',
        'ILS': 'ILS',
        'ILS-ES': 'ILS-SA', # Renombrado para que sea consistente (ES = Enfriamiento Simulado = SA)
        'GRASP': 'GRASP',
        'AM-Best': 'AM-Best'
    })

    datasets = df['Dataset'].unique()
    ks = df['K'].unique()
    
    # --- 2. LISTAS ORDENADAS PARA CADA COMPARATIVA ---
    # Todos los algoritmos para ver el rango completo
    comp_1_global = ['Random', 'Greedy', 'Local', 'SA', 'SA-Fast', 'BMB', 'ILS', 'ILS-SA', 'GRASP', 'AM-Best']
    
    # ¿Cómo mejoran los mecanismos de escape a la BL pura?
    comp_2_trayectorias = ['Local', 'SA', 'SA-Fast', 'ILS', 'ILS-SA']
    
    # Arranque aleatorio vs Arranque Inteligente
    comp_3_multiarranque = ['BMB', 'GRASP']
    
    # Los mejores de trayectorias frente al memético poblacional
    comp_4_titanes = ['Local', 'ILS', 'GRASP', 'AM-Best']

    for dataset in datasets:
        for k in ks:
            subset = df[(df['Dataset'] == dataset) & (df['K'] == k)]
            
            if subset.empty:
                continue
                
            name = dataset.split('/')[-1] 
            
            # --- COMPARATIVA 1: GLOBAL ---
            generate_boxplot(subset, comp_1_global,
                             f"{name} (k={k}) - Comparativa Global",
                             f"{name}_k{k}_1_global.png", output_dir,
                             figsize=(12, 6), rotate_x=True)

            # --- COMPARATIVA 2: ZOOM TRAYECTORIAS ---
            # Ideal para apoyar el Wilcoxon 1 y 2 (Local vs SA y Local vs ILS)
            generate_boxplot(subset, comp_2_trayectorias,
                             f"{name} (k={k}) - Búsqueda Local y Mecanismos de Escape",
                             f"{name}_k{k}_2_trayectorias.png", output_dir,
                             figsize=(8, 5))

            # --- COMPARATIVA 3: ZOOM MULTI-ARRANQUE ---
            # Ideal para apoyar el Wilcoxon 3 (BMB vs GRASP)
            generate_boxplot(subset, comp_3_multiarranque,
                             f"{name} (k={k}) - Multi-arranque: Aleatorio vs Guiado",
                             f"{name}_k{k}_3_multiarranque.png", output_dir,
                             figsize=(5, 5))

            # --- COMPARATIVA 4: EL CHOQUE DE TITANES ---
            # Ideal para apoyar el Wilcoxon 4 (ILS vs AM-Best)
            # Metemos LocalSearch como referencia (baseline)
            generate_boxplot(subset, comp_4_titanes,
                             f"{name} (k={k}) - Trayectorias Avanzadas vs Memético",
                             f"{name}_k{k}_4_titanes.png", output_dir,
                             figsize=(7, 5))

if __name__ == '__main__':
    main()