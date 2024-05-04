#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>

#define SHM_SIZE 1024  
#define FILENAME "file.txt"

// Global variables for widgets
GtkWidget *timeSliceLabel;
GtkWidget *timeSliceSlider;
GtkWidget *pathEntry;
GtkWidget *window;
GtkWidget *submit;
GtkWidget *comboBoxLabel;
GtkWidget *comboBox;
GtkWidget *pathLabel;
GtkWidget *fixed;
pid_t pid;
int shmid;
void *shmaddr;

char* reader() {
    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1) {
        perror("Error in attach in reader");
        exit(EXIT_FAILURE);
    }
    printf("\nGUI: Shared memory attached at address %p\n", shmaddr);
    return (char *)shmaddr;

}
void writer() {
    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1) {
        perror("Error in attach in writer");
        exit(EXIT_FAILURE);
    }
    strcpy((char *)shmaddr, "0"); 
}

// Function to handle the change in the algorithm selection
void combo_box_changed(GtkComboBox *combo_box, gpointer user_data) {
    gint index = gtk_combo_box_get_active(combo_box);

    // Show or hide time slice widgets based on the selected algorithm
    if (index == 1) {
        gtk_widget_show(timeSliceLabel);
        gtk_widget_show(timeSliceSlider);
    } else {
        gtk_widget_hide(timeSliceLabel);
        gtk_widget_hide(timeSliceSlider);
    }
}


gboolean showButton(gpointer data) {
    char buttonData = reader()[0];
    if(buttonData == '1'){
        printf("%c\n", buttonData); // Print character using %c
        gtk_widget_set_sensitive(submit, TRUE);
        void *shmaddr = shmat(shmid, NULL, 0);
        if (shmaddr == (void *)-1) {
            perror("Error in attach in writer");
            exit(EXIT_FAILURE);
        }
        shmdt(shmaddr);
        shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
    }
        
    // Return TRUE to keep the timeout function running
    return TRUE;
}


// Function to handle the change in the time slice value
void time_slice_changed(GtkAdjustment *adjustment, gpointer user_data) {
    gdouble value = gtk_adjustment_get_value(adjustment);
    g_print("Time Slice: %.1f\n", value);
}

// Function to handle the submit button click event
void submit_handler(GtkWidget *widget, gpointer data) {
    // Retrieve path, algorithm index, and time slice value from widgets
    const gchar *path = gtk_entry_get_text(GTK_ENTRY(pathEntry));
    gint algoNo = gtk_combo_box_get_active(GTK_COMBO_BOX(comboBox));
    gdouble timeSliceValue = gtk_range_get_value(GTK_RANGE(timeSliceSlider));
    
    // Hide the submit button while processing

    writer();
    gtk_widget_set_sensitive(submit, FALSE);

    // Convert algorithm option and path text to strings
    char algo_arg[50];
    char path_arg[100];
    char time_slice_arg[10];
    snprintf(algo_arg, sizeof(algo_arg), "%i", algoNo);
    snprintf(path_arg, sizeof(path_arg), "%s", path);
    snprintf(time_slice_arg, sizeof(time_slice_arg), "%.0f", timeSliceValue);
    // Asynchronously spawn the child process
    gboolean success = g_spawn_async(NULL,                  // working_directory
                                     (char *[]){ "./process_generator.out", algo_arg, path_arg, time_slice_arg, NULL }, // argv
                                     NULL,                  // envp
                                     G_SPAWN_DEFAULT,      // flags
                                     NULL,                  // child_setup
                                     NULL,                  // user_data
                                     &pid,                 // child_pid
                                     NULL);                 // error
    if (!success) {
        // Handle error
        g_print("Failed to spawn child process\n");
        // Show the submit button again after processing
        gtk_widget_set_sensitive(submit, TRUE);
    }
}


// Function to handle window destruction
void destroy_handler(GtkWidget *widget, gpointer data) {
    // Terminate the child process if it's running

    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1) {
        perror("Error in attach in writer");
        exit(EXIT_FAILURE);
    }
    //shmdt(shmaddr);
    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
    if (pid > 0)
        kill(pid, SIGINT);
    // Quit GTK main loop
    gtk_main_quit();
    exit(0);
}

int main(int argc, char *argv[]) {

    signal(SIGINT, SIG_IGN);
    
    key_t key = 50;  // Generate a key for the shared memory segment

    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    // Create a shared memory segment
    if (shmid < 0 ){
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    printf("Shared memory created with id %d\n", shmid);
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    // Create combo box and submit button
    comboBox = gtk_combo_box_text_new();
    submit = gtk_button_new_with_label("submit");

    // Create labels and entry for path and time slice
    comboBoxLabel = gtk_label_new("Select the Algorithm");
    fixed = gtk_fixed_new();
    timeSliceLabel = gtk_label_new("Time Slice:");
    timeSliceSlider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 15, 1);
    pathLabel = gtk_label_new("Path:");
    pathEntry = gtk_entry_new();

    // Customize time slice slider
    gtk_scale_set_digits(GTK_SCALE(timeSliceSlider), 0);
    gtk_scale_set_value_pos(GTK_SCALE(timeSliceSlider), GTK_POS_LEFT);
    gtk_scale_set_draw_value(GTK_SCALE(timeSliceSlider), TRUE);
    gtk_widget_set_size_request(timeSliceSlider, 150, -1);
    gtk_adjustment_set_value(gtk_range_get_adjustment(GTK_RANGE(timeSliceSlider)), 0);
    g_signal_connect(GTK_ADJUSTMENT(gtk_range_get_adjustment(GTK_RANGE(timeSliceSlider))), "value_changed", G_CALLBACK(time_slice_changed), NULL);

    // Set default path for entry
    gtk_entry_set_text(GTK_ENTRY(pathEntry), "TestCases/processes.txt");

    // Populate combo box with algorithm options
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboBox), "STRN");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboBox), "RR");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboBox), "HPF");
    gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), 0);

    // Set size for combo box
    gtk_widget_set_size_request(comboBox, 100, 35);

    // Connect signals for combo box and submit button
    g_signal_connect(comboBox, "changed", G_CALLBACK(combo_box_changed), timeSliceLabel);
    g_signal_connect(submit, "clicked", G_CALLBACK(submit_handler), NULL);

    // Set width for path entry
    gtk_entry_set_width_chars(GTK_ENTRY(pathEntry), 25);

    // Add widgets to the fixed container
    gtk_fixed_put(GTK_FIXED(fixed), comboBoxLabel, 40, 25);
    gtk_fixed_put(GTK_FIXED(fixed), comboBox, 200, 20);
    gtk_fixed_put(GTK_FIXED(fixed), timeSliceLabel, 40, 130);
    gtk_fixed_put(GTK_FIXED(fixed), timeSliceSlider, 200, 130);
    gtk_fixed_put(GTK_FIXED(fixed), pathLabel, 40, 85);
    gtk_fixed_put(GTK_FIXED(fixed), pathEntry, 100, 80);
    gtk_fixed_put(GTK_FIXED(fixed), submit, 350, 200);

    // Add fixed container to the window
    gtk_container_add(GTK_CONTAINER(window), fixed);

    // Set window properties
    gtk_window_set_title(GTK_WINDOW(window), "Scheduler");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);

    // Connect signal for window destruction
    g_signal_connect(window, "destroy", G_CALLBACK(destroy_handler), NULL);

    g_timeout_add(1000, showButton, submit);

    // Show all widgets and start GTK main loop
    gtk_widget_show_all(window);
    gtk_widget_hide(timeSliceLabel);
    gtk_widget_hide(timeSliceSlider);
    gtk_main();

    return 0;
}