import matplotlib
matplotlib.use("Agg")
from matplotlib import pyplot as plt

def read_values_from_file(filename):
    # Open file
    with open(filename, "r") as file:
        content = file.read()  
    # Split values by ";"
    string_values = content.split(";")
    values = []
    # Append values as integer to list
    for value in string_values:
        value = value.strip()
        if value:
            values.append(int(value))
    return values

def create_diagram(values):
    # X-axe from 0 to len(values)
    x = list(range(len(values)))

    # Plot values to diagram
    plt.plot(x, values, marker="o", linestyle="-", color="orange", label="TSC difference")
    
    plt.xlabel("Measures (every 5s)")
    plt.ylabel("TSC difference (ns)")
    plt.title("TSC drift on different cores")
    plt.legend()
    plt.grid(True)
    plt.ylim(bottom=0, top=max(100, max(values)))
    plt.savefig("diagram.pdf") 
    plt.close()

if __name__ == "__main__":
    src_path = "../test_results/output.txt"
    values = read_values_from_file(src_path)
    create_diagram(values)
