#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 

static void activate(GtkApplication *app, gpointer user_data);
static void button_clicked(GtkButton *button, gpointer user_data);
static void algo_combo_changed(GtkComboBox *combo_box, gpointer user_data);

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    GtkApplication *app;
    app = gtk_application_new("in.aducators", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Scheduler");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 400);

    GtkWidget *box;
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Create a label for the first input field (algorithm)
    GtkWidget *algorithm_label;
    algorithm_label = gtk_label_new("Algorithm:");
    gtk_box_pack_start(GTK_BOX(box), algorithm_label, FALSE, FALSE, 0);

    // Create a combo box for the first input field
    GtkWidget *algorithm_combo;
    algorithm_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algorithm_combo), "FCFS");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algorithm_combo), "RR");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algorithm_combo), "HPF");
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), 0);
    gtk_box_pack_start(GTK_BOX(box), algorithm_combo, FALSE, FALSE, 0);
    
    // Connect callback for combo box changed signal
    g_signal_connect(algorithm_combo, "changed", G_CALLBACK(algo_combo_changed), box);

    // Create a label for the second input field (path)
    GtkWidget *path_label;
    path_label = gtk_label_new("Path:");
    gtk_box_pack_start(GTK_BOX(box), path_label, FALSE, FALSE, 0);

    // Create an entry field for the second input field
    GtkWidget *path_entry;
    path_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), path_entry, FALSE, FALSE, 0);

    // Create the "Time Slice" label
    GtkWidget *time_slice_label;
    time_slice_label = gtk_label_new("Time Slice:");
    gtk_box_pack_start(GTK_BOX(box), time_slice_label, FALSE, FALSE, 0);

    // Create the "Time Slice" scale (slider)
    GtkWidget *time_slice_scale;
    time_slice_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 100, 1);
    gtk_scale_set_value_pos(GTK_SCALE(time_slice_scale), GTK_POS_RIGHT);
    gtk_scale_set_digits(GTK_SCALE(time_slice_scale), 0); // Set number of decimal places
    gtk_box_pack_start(GTK_BOX(box), time_slice_scale, FALSE, FALSE, 0);
    gtk_widget_hide(time_slice_scale); // Hide initially

    // Create the "Submit" button
    GtkWidget *submit_button;
    submit_button = gtk_button_new_with_label("Submit");
    gtk_box_pack_start(GTK_BOX(box), submit_button, FALSE, FALSE, 0);
    g_signal_connect(submit_button, "clicked", G_CALLBACK(button_clicked), algorithm_combo);

    // Set user data for the path entry field and time slice scale
    g_object_set_data(G_OBJECT(submit_button), "path_entry", path_entry);
    g_object_set_data(G_OBJECT(submit_button), "time_slice_scale", time_slice_scale);

    gtk_widget_show_all(window);
}

static void algo_combo_changed(GtkComboBox *combo_box, gpointer user_data) {
    // Get the selected algorithm
    const gchar *selected_algo = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_box));

    // Get the box containing the widgets
    GtkWidget *box = GTK_WIDGET(user_data);

    // Find the "Time Slice" scale (if exists)
    GtkWidget *time_slice_scale = NULL;

    // Check if the box is a GtkBox
    if (GTK_IS_BOX(box)) {
        // Get the last child of the box
        GList *children = gtk_container_get_children(GTK_CONTAINER(box));
        if (children != NULL) {
            time_slice_scale = GTK_WIDGET(g_list_last(children)->data);
            g_list_free(children);
        }
    }

    // Show/hide the scale based on the selected algorithm
    if (time_slice_scale != NULL) {
        if (g_strcmp0(selected_algo, "RR") == 0) {
            gtk_widget_show(time_slice_scale);
        } else {
            gtk_widget_hide(time_slice_scale);
        }
    }
}


static void button_clicked(GtkButton *button, gpointer user_data) {
    // Retrieve the algorithm option from the combo box
    GtkComboBox *algorithm_combo = GTK_COMBO_BOX(user_data);
    const gchar *algorithm_option = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(algorithm_combo));

    // Retrieve the text from the path entry field
    GtkEntry *path_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "path_entry"));
    const gchar *path_text = gtk_entry_get_text(path_entry);

    // Retrieve the value from the time slice scale
    GtkScale *time_slice_scale = GTK_SCALE(g_object_get_data(G_OBJECT(button), "time_slice_scale"));
    gdouble time_slice_value = gtk_range_get_value(GTK_RANGE(time_slice_scale));

    int algoNo = 1;
    if(algorithm_option=="RR"){
        algoNo = 2;
    }else if(algorithm_option=="HPF"){
        algoNo = 3;
    }
    // Print the inputs
    g_print("Algorithm: %s\n", algorithm_option);
    g_print("Path: %s\n", path_text);
    g_print("Time Slice: %.0f\n", time_slice_value);

    // Fork a child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        // Convert algorithm_option and path_text to strings
        char algo_arg[50];
        char path_arg[100];
        char time_slice_arg[10];
        snprintf(algo_arg, sizeof(algo_arg), "%i", algoNo);
        snprintf(path_arg, sizeof(path_arg), "%s", path_text);
        snprintf(time_slice_arg, sizeof(time_slice_arg), "%.0f", time_slice_value);

        // Execute process_generator.c in the child process with arguments
        execl("./process_generator.out", "./process_generator.out", algo_arg, path_arg, time_slice_arg, NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }
    
    // Free the memory allocated for the combo box option
    g_free((gpointer)algorithm_option);
}
