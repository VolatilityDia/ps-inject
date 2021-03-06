#include <getopt.h>
#include <stdio.h>

#include "main.h"

#include "common.h"
#include "file.h"
#include "str.h"
#include "mem.h"
#include "inject.h"




static const char short_options[]= "f:s:n";
static const struct option long_options[]={
	{"sc-file",           required_argument, NULL, 'f'},
	{"sc-string",         required_argument, NULL, 's'},
	{"no-restore",        no_argument,       NULL, 'n'},
	{NULL, 0, NULL, 0}
};

void parser_args(int *argc, char ***argv, inject_options_t *opt){

	int optc;
	int option_index = 0;

	while((optc = getopt_long(*argc, *argv, short_options, long_options, &option_index))
		!= -1){

		switch(optc){

			case 'f':
				opt->filename = optarg;
				break;

			case 's':
				opt->shellcode = optarg;
				break;

			case 'n':
				opt->no_restore = 1;
				break;

			case '?':
				exit(1);

			default:
				help();

		}


	}

	if(optind+1 != *argc){
		help();
	}

	opt->target_pid.number = parser_pid( (*argv)[optind] );
	opt->target_pid.str = (*argv)[optind];
}

void banner(void){
	static const char ascii_banner[]=
		" ____    ____     __  __ _    __  ____  ___  ____ \n"
		"(  _ \\  / ___)   (  )(  ( \\ _(  )(  __)/ __)(_  _)\n"
		" ) __/_ \\___ \\ _  )( /    // \\) \\ ) _)( (__   )(  \n"
		"(__) (_)(____/(_)(__)\\_)__)\\____/(____)\\___) (__) \n";

	puts(ascii_banner);

}

void help(void){
	printf("Usage: ps-inject [OPTIONS] [PID]\n\n");
	printf("   -f, --sc-file FILE       File contains shellcode bytes\n");
	printf("   -s, --sc-string STRING   Shellcode string, e.g '\\x90\\x90\\x90'\n");
	printf("   -n, --no-restore         Not restore memory overwrited by shellcode\n\n");
	exit(0);
}




int inject_code(inject_options_t *opts){
	maped_file_t maped_file = maped_file_default;
	bytecode_string_t sc = bytecode_string_default;

	if(opts->filename){
		info("checking file => %s\n", opts->filename);
		memorymap(opts->filename, &maped_file);
		ptrace_inject(maped_file.ptr, maped_file.size, opts->target_pid, opts->no_restore);
		memorymapfree(&maped_file);
	}

	if(opts->shellcode){
		info("checking shellcode string...\n");
		str2bytecode(opts->shellcode, &sc);
		ptrace_inject(sc.ptr, sc.len, opts->target_pid, opts->no_restore);
		xfree(sc.ptr);
	}

	return 0;
}

int main(int argc, char **argv){
	inject_options_t options = inject_options_default;

	banner();
	parser_args(&argc, &argv, &options);

	return inject_code(&options);
}
