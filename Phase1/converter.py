import pandas as pd
import matplotlib.pyplot as plt

def read_text_file(file_path):
    """
    Read a text file and return its lines.
    Args:
        file_path (str): Path to the text file.
    Returns:
        list: List of lines from the text file.
    """
    with open(file_path, 'r') as file:
        lines = file.readlines()
    return lines

def create_dataframe_from_perf_file(file_path):
    """
    Create a DataFrame from a performance text file.
    Args:
        file_path (str): Path to the performance text file.
    Returns:
        pandas.DataFrame: DataFrame created from the text file.
    """
    lines = read_text_file(file_path)
    if not lines:
        return None
    data = [line.strip().split('\t') for line in lines]
    df = pd.DataFrame(data, columns=['Metric', 'Value'])
    df['Value'] = pd.to_numeric(df['Value'], errors='ignore')
    return df

def create_dataframe_from_log_file(file_path):
    """
    Create a DataFrame from a log text file.
    Args:
        file_path (str): Path to the log text file.
    Returns:
        pandas.DataFrame: DataFrame created from the text file.
    """
   
    data = {'At time':[], 'process' : [], 'status' : [], 'arrive' : [], 'total' : [], 'remain' : [], 'wait' : [], 'TA' : [], 'WTA' : []}
    
    lines = read_text_file(file_path)

    if not lines:
        return pd.DataFrame()
    
    for line in lines:
        parts = line.strip().split('\t')
        data['At time'].append(parts[parts.index('At time') + 1])
        data['process'].append(parts[parts.index('process') + 1])
        data['status'].append(parts[parts.index('process') + 2])
        data['arrive'].append(parts[parts.index('arrive') + 1])
        data['total'].append(parts[parts.index('total') + 1])
        data['remain'].append(parts[parts.index('remain') + 1])
        data['wait'].append(parts[parts.index('wait') + 1])
        TA = parts[parts.index('TA') + 1] if 'TA' in parts else None
        data['TA'].append(TA)
        WTA = parts[parts.index('WTA') + 1] if 'WTA' in parts else None
        data['WTA'].append(WTA)
    df = pd.DataFrame(data)
    return df 

def plot_dataframe_as_table(df, image_path, dpi=500, font_size=8, font_family='Arial', cell_pad=0.1):
    """
    Plot a DataFrame as a table and save it as an image file with increased resolution.
    Args:
        df (pandas.DataFrame): DataFrame to be plotted.
        image_path (str): Path to save the image file.
        dpi (int): Dots per inch for the resolution of the image (default is 500).
        font_size (int): Font size for the table cells (default is 12).
        font_family (str): Font family for the table cells (default is 'Arial').
        cell_pad (float): Padding for the cells (default is 0.1).
    """ 
    if df.empty:
        print("DataFrame is empty. Cannot plot table.")
        return

    fig, ax = plt.subplots(figsize=(12, 10))
    ax.axis('tight')
    ax.axis('off')
    
    table = ax.table(cellText=df.values, colLabels=df.columns, loc='center', 
                     cellLoc='center', fontsize=font_size, 
                     colColours=['lightgray'] * len(df.columns),
                     colWidths=[0.15] * len(df.columns))
    
    for (row, col), cell in table.get_celld().items():
        if (row == 0):
            cell.set_text_props(fontsize=font_size + 2, fontweight='bold')
    
    table.auto_set_font_size(False)
    table.set_fontsize(font_size)
    table.auto_set_column_width(col=list(range(len(df.columns))))
    
    table.set_fontsize(font_size)
    plt.savefig(image_path, bbox_inches='tight', pad_inches=0.1, dpi=dpi)
    #plt.show();

if __name__ == "__main__":
    try:
        # File paths
        log_file_path = 'outputs/scheduler.log'
        perf_file_path = 'outputs/scheduler.perf'

        # Process log file
        log_df = create_dataframe_from_log_file(log_file_path)
        plot_dataframe_as_table(log_df, 'outputs/logImage.png')
        
        # Process performance file
        perf_df = create_dataframe_from_perf_file(perf_file_path)
        plot_dataframe_as_table(perf_df, 'outputs/perfImage.png')

    except KeyboardInterrupt:
        print("Execution interrupted by the user.")

