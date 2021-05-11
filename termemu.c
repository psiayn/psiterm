#include <vte/vte.h>
#define CLR_R(x)   (((x) & 0xff0000) >> 16)
#define CLR_G(x)   (((x) & 0x00ff00) >>  8)
#define CLR_B(x)   (((x) & 0x0000ff) >>  0)
#define CLR_16(x)  ((double)(x) / 0xff)
#define CLR_GDK(x) (const GdkRGBA){ .red = CLR_16(CLR_R(x)), \
								.green = CLR_16(CLR_G(x)), \
								.blue = CLR_16(CLR_B(x)), \
								.alpha = 0 }

static gboolean
on_title_changed(GtkWidget *terminal, gpointer user_data) {
	GtkWindow *window = user_data;
	gtk_window_set_title(window, 
			vte_terminal_get_window_title(VTE_TERMINAL(terminal)?:"terminal"));
	return TRUE;
}

static gboolean
on_key_pressed(GtkWidget *terminal, GdkEventKey *event, gpointer user_data) {
	switch(event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK)) {
		case GDK_CONTROL_MASK | GDK_SHIFT_MASK:
			switch (event->keyval) {
				case GDK_KEY_V:
					vte_terminal_paste_clipboard(VTE_TERMINAL(terminal));
					return TRUE;
				case GDK_KEY_C:
					vte_terminal_copy_clipboard(VTE_TERMINAL(terminal));
					return TRUE;
			}
	}
	return FALSE;
}

static void
child_ready(VteTerminal *terminal, GPid pid, GError *error, gpointer user_data){
	if (!terminal) return;
	if (pid == -1) gtk_main_quit();
}
int
main(int argc, char *argv[]){
	GtkWidget *window, *terminal;

	/* Initialise GTK, the window and the terminal */
	gtk_init(&argc, &argv);
	terminal = vte_terminal_new();
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "psiterm");

	/* Start a new shell */
	gchar **envp = g_get_environ();
	gchar **command = (gchar *[]){g_strdup(g_environ_getenv(envp, "SHELL")), NULL };
	g_strfreev(envp);
	vte_terminal_spawn_async(VTE_TERMINAL(terminal),
		VTE_PTY_DEFAULT,
		NULL,         /* working directory  */
		command,      /* command */
		NULL,         /* environment */
		0,            /* spawn flags */
		NULL, NULL,   /* child setup */
		NULL,         /* child pid */
		-1,           /* timeout */
		NULL,         /* cancellable */
		child_ready,  /* callback */
		NULL);        /* user_data */

	vte_terminal_set_colors(VTE_TERMINAL(terminal),
		&CLR_GDK(0xffffff),
		&(GdkRGBA){ .alpha = 0.85 },
		(const GdkRGBA[]){
		CLR_GDK(0x111111),
		CLR_GDK(0xd36265),
		CLR_GDK(0xaece91),
		CLR_GDK(0xe7e18c),
		CLR_GDK(0x5297cf),
		CLR_GDK(0x963c59),
		CLR_GDK(0x5E7175),
		CLR_GDK(0xbebebe),
		CLR_GDK(0x666666),
		CLR_GDK(0xef8171),
		CLR_GDK(0xcfefb3),
		CLR_GDK(0xfff796),
		CLR_GDK(0x74b8ef),
		CLR_GDK(0xb85e7b),
		CLR_GDK(0xA3BABF),
		CLR_GDK(0xffffff)
	}, 16);
	
	vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal), 0);
	vte_terminal_set_scroll_on_output(VTE_TERMINAL(terminal), FALSE);
	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(terminal), TRUE);
	vte_terminal_set_mouse_autohide(VTE_TERMINAL(terminal), TRUE);

	/* Connect some signals */
	g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
	g_signal_connect(terminal, "child-exited", gtk_main_quit, NULL);
	g_signal_connect(terminal, "window-title-changed", 
			G_CALLBACK(on_title_changed), GTK_WINDOW(window));
	g_signal_connect(terminal, "key-press-event", 
			G_CALLBACK(on_key_pressed), GTK_WINDOW(window));


	/* Put widgets together and run the main loop */
	gtk_container_add(GTK_CONTAINER(window), terminal);
	gtk_widget_show_all(window);
	gtk_main();
}
