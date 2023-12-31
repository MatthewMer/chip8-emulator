#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <conio.h>


#include "SDL.h"
#include "SDL_opengl.h"
#include "Chip8.h"
#include "defs.h"
#include "config.h"


void print_usage(char** argv, int j);
int print_params(int argc, char** argv, u8& params);
void print_info(char* file, Chip8 cpu, std::vector<char> buf);
static float crt(float x, float y, float x_centre, float y_centre);
void audio_callback(void* userdata, uint8_t* stream, int len);

// screen
const int x_centre = CHIP8_WIDTH_HALF * CHIP8_WIN_MULT;
const int y_centre = CHIP8_HEIGHT_HALF * CHIP8_WIN_MULT;

// audio
static const float volume = 0.05;
static const float frequency = 200.0;

const char* err_msg[] = {
    "Memory: index out of bounds\n",
    "Stack: index out of bounds\n",
    "Screen: Pixel out of bounds (max: h:64, v:32)\n"
};


int main(int argc, char** argv) {

    u8 params = 0x00;
    int errors = print_params(argc, argv, params);
    if (errors) return errors;

    // load file -----
    const char* file_name = argv[argc - 1];
    printf("\nFile to load: %s\n", file_name);

    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (!file) {
        printf("\nFailed to open file/No proper file\n");
        return 1;
    }

    std::vector<char> buf(size);
    if (!file.read(buf.data(), size)) {
        printf("\nFailed to read file\n");
        return 1;
    }

    file.close();

    // init emulator -----
    Chip8 cpu = Chip8();
    cpu.load(buf);

    if (params & 0x1) print_info(argv[argc - 1], cpu, buf);
    bool step = (params & 0x2);
    
    char key;
    int vkey;

    // init screen -----
    SDL_Init(SDL_INIT_EVERYTHING);

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_Window* window = SDL_CreateWindow(
        EMU_WIN_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        CHIP8_WIDTH * CHIP8_WIN_MULT,
        CHIP8_HEIGHT * CHIP8_WIN_MULT,
        /*SDL_WINDOW_OPENGL |*/ SDL_WINDOW_SHOWN
    );
    //SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);

    // init audio -----
    uint64_t samples_played = 0;

    static SDL_AudioSpec audio_spec_want, audio_spec;
    SDL_zero(audio_spec_want);
    SDL_zero(audio_spec);

    audio_spec_want.freq = 44100;
    audio_spec_want.format = AUDIO_F32;
    audio_spec_want.channels = 2;
    audio_spec_want.samples = 512;
    audio_spec_want.callback = audio_callback;
    audio_spec_want.userdata = (void*)&samples_played;

    SDL_AudioDeviceID audio_device_id = SDL_OpenAudioDevice(
        NULL, 0,
        &audio_spec_want, &audio_spec,
        SDL_AUDIO_ALLOW_FORMAT_CHANGE
    );

    // precalculate pixels
    const SDL_Color pixel_color = {
        0x0, 0xee, 0x88, 0x0
    };
    const int pixel_margin = 1;

    int x_centre = CHIP8_WIDTH * CHIP8_WIN_MULT / 2;
    int y_centre = CHIP8_HEIGHT * CHIP8_WIN_MULT / 2;

    std::vector<std::vector<SDL_Vertex>> pixel_triangles = std::vector<std::vector<SDL_Vertex>>();
    for (int x = 0; x < CHIP8_WIDTH; x++) {
        for (int y = 0; y < CHIP8_HEIGHT; y++) {
            float x_offset = x * CHIP8_WIN_MULT + 1;
            float y_offset = y * CHIP8_WIN_MULT + 1;

            // first triangle
            std::vector<SDL_Vertex> verts = {
                { SDL_FPoint{ crt(x_offset + pixel_margin, y_offset + pixel_margin, x_centre, y_centre), crt(y_offset + pixel_margin, x_offset + pixel_margin, y_centre, x_centre)}, pixel_color, SDL_FPoint{0},},
                { SDL_FPoint{ crt(x_offset + pixel_margin, y_offset + CHIP8_WIN_MULT - pixel_margin, x_centre, y_centre), crt(y_offset + CHIP8_WIN_MULT - pixel_margin, x_offset + pixel_margin, y_centre, x_centre)}, pixel_color, SDL_FPoint{ 0 }, },
                { SDL_FPoint{ crt(x_offset + CHIP8_WIN_MULT - pixel_margin, y_offset + pixel_margin, x_centre, y_centre), crt(y_offset + pixel_margin, x_offset + CHIP8_WIN_MULT - pixel_margin, y_centre, x_centre)}, pixel_color, SDL_FPoint{ 0 }, },
            };
            
            /*
            std::vector<SDL_Vertex> verts = {
                { SDL_FPoint{ x_offset + pixel_margin, y_offset + pixel_margin}, pixel_color, SDL_FPoint{0},},
                { SDL_FPoint{ x_offset + pixel_margin, y_offset + CHIP8_WIN_MULT - pixel_margin}, pixel_color, SDL_FPoint{ 0 }, },
                { SDL_FPoint{ x_offset + CHIP8_WIN_MULT - pixel_margin, y_offset + pixel_margin}, pixel_color, SDL_FPoint{ 0 }, },
            };*/

            // second triangle
            std::vector<SDL_Vertex> verts_ = {
                { SDL_FPoint{ crt(x_offset + CHIP8_WIN_MULT - pixel_margin, y_offset + pixel_margin, x_centre, y_centre), crt(y_offset + pixel_margin, x_offset + CHIP8_WIN_MULT - pixel_margin, y_centre, x_centre)}, pixel_color, SDL_FPoint{0},},
                { SDL_FPoint{ crt(x_offset + pixel_margin, y_offset + CHIP8_WIN_MULT - pixel_margin, x_centre, y_centre), crt(y_offset + CHIP8_WIN_MULT - pixel_margin, x_offset + pixel_margin, y_centre, x_centre)}, pixel_color, SDL_FPoint{0},},
                { SDL_FPoint{ crt(x_offset + CHIP8_WIN_MULT - pixel_margin, y_offset + CHIP8_WIN_MULT - pixel_margin, x_centre, y_centre), crt(y_offset + CHIP8_WIN_MULT - pixel_margin, x_offset + CHIP8_WIN_MULT - pixel_margin, y_centre, x_centre)}, pixel_color, SDL_FPoint{0},},
            };

            /*
            std::vector<SDL_Vertex> verts_ = {
                { SDL_FPoint{ x_offset + CHIP8_WIN_MULT - pixel_margin, y_offset + pixel_margin }, pixel_color, SDL_FPoint{ 0 }, },
                { SDL_FPoint{ x_offset + pixel_margin, y_offset + CHIP8_WIN_MULT - pixel_margin }, pixel_color, SDL_FPoint{ 0 }, },
                { SDL_FPoint{ x_offset + CHIP8_WIN_MULT - pixel_margin, y_offset + CHIP8_WIN_MULT - pixel_margin }, pixel_color, SDL_FPoint{ 0 }, },
            };*/

            pixel_triangles.push_back(verts);
            pixel_triangles.push_back(verts_);
        }
    }

    // execution -----
    printf("\n ----- INSTRUCTION EXECUTION -----\n\n");
    errors = 0x00;
    bool sound_started = false;

    while (!errors) {

        // handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type){
            case SDL_QUIT:
                SDL_DestroyWindow(window);
                return 0;
                break;
            case SDL_KEYUP:
                key = event.key.keysym.sym;
                vkey = cpu.keys.get_mapped_key(key);
                if (vkey != -1) {
                    cpu.keys.key_down(vkey);
                }
                break;
            }
        }

        // execute
        errors = cpu.exec();

        // playback audio
        if (cpu.sound || cpu.awaits_key) {
            if (!sound_started) {
                SDL_PauseAudioDevice(audio_device_id, 0);
                sound_started = true;
            }
        }
        else {
            SDL_PauseAudioDevice(audio_device_id, 1);
            sound_started = false;
        }

        // render screen
        if (cpu.render) {
            SDL_SetRenderDrawColor(renderer, 0x0, 0x11, 0x0, 0x0);          // Red, Green, Blue, Alpha
            SDL_RenderClear(renderer);

            //SDL_SetRenderDrawColor(renderer, 0x0, 0xff, 0x99, 0x0);
            for (int x = 0; x < CHIP8_WIDTH; x++) {
                for (int y = 0; y < CHIP8_HEIGHT; y++) {
                    if (cpu.screen.isset(x, y)) {
                        int tris_offset = (x * CHIP8_HEIGHT + y) * 2;
                        SDL_RenderGeometry(renderer, nullptr, pixel_triangles[tris_offset].data(), pixel_triangles[tris_offset].size(), nullptr, 0);
                        SDL_RenderGeometry(renderer, nullptr, pixel_triangles[tris_offset + 1].data(), pixel_triangles[tris_offset + 1].size(), nullptr, 0);
                    }
                }
            }
            SDL_RenderPresent(renderer);
            cpu.render = false;
        }

        if (step) char c = _getch();
    }

    SDL_DestroyWindow(window);

    printf("\nExitcode: 0x%.2x\n\n", errors);
    return errors;
}


void print_usage(char** argv, int j) {
    printf("\nERROR: No proper parameter recognized at position %i:\n", j);
    for (int k = 0; argv[k]; k++) {
        printf("%s ", argv[k]);
    }
    std::cout << "\n";
    for (int k = 0; k < j; k++) {
        if (argv[k]) {
            for (int i = 0; i < strlen(argv[k]); i++) {
                std::cout << " ";
            }
        }
    }
    std::cout << " ^\n\nType \"Chip8.exe -h\" or \"Chip8.exe --help\" for more information about usage\n\n";
}

int print_params(int argc, char** argv, u8& params) {
    if (argc < 2 || (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))) {
        printf("\nUsage:\n\tChip8.exe <options> \"\\path\\to\\file\\to\\load\"\n\n");
        printf("Options:\n\t--verbose/-v\t\tPrint additional information to stdout\n");
        printf("\t--help/-h\t\tShow information aboout proper usage\n");
        printf("\t--step/-s\t\tExecute instructions step by step\n");

        return 1;
    }
    else if (argc > 2) {
        for (int j = 1; argv[j + 1]; j++) {

            // no proper parameter
            if (*(*(argv + j)) != '-') {
                print_usage(argv, j);
                return 1;
            }

            // verbose -----
            if (strcmp(argv[j], "-v") == 0 || strcmp(argv[j], "--verbose") == 0) {
                params |= 0x1;
            }
            else if (strcmp(argv[j], "-s") == 0 || strcmp(argv[j], "--step") == 0) {
                params |= 0x2;
            }
            // no proper parameter
            else if (strcmp(argv[j], "-h") == 0 || strcmp(argv[j], "--help") == 0) {
                printf("\nWrong Usage\n\nType \"Chip8.exe -h\" or \"Chip8.exe --help\" for more information about usage\n\n");
                return 1;
            }
        }
        if (*(*(argv + argc - 1)) == '-') {
            printf("\nA parameter is no file\n\nType \"Chip8.exe -h\" or \"Chip8.exe --help\" for more information about usage\n\n");
            return 1;
        }
    }
    else if (argc == 2) {
        if (*(*(argv + 1)) == '-') {
            printf("\nA parameter is no file\n\nType \"Chip8.exe -h\" or \"Chip8.exe --help\" for more information about usage\n\n");
            return 1;
        }
    }

    return 0;
}


static float crt(float x, float y, float x_centre, float y_centre) {
    float x_ = x - x_centre;
    float y_percent = (y - y_centre) / y_centre;
    return (x_ * (-0.03 * pow(y_percent, 2) + 1)) * (1.f - (CHIP8_SCREEN_BORDER_PADDING / 100.f)) + x_centre;
}

void audio_callback(void* userdata, uint8_t* stream, int len)
{
    uint64_t* samples_played = (uint64_t*)userdata;
    float* fstream = (float*)(stream);

    for (int sid = 0; sid < (len / 8); ++sid)
    {
        double time = (*samples_played + sid) / 44100.0;
        double x = 2.0 * M_PI * time * frequency;
        fstream[2 * sid + 0] = volume * (sin(x) > 0 ? 1.0 : -1.0); /* L */
        fstream[2 * sid + 1] = fstream[2 * sid + 0]; /* R */
    }

    *samples_played += (len / 8);
}


void print_info(char* file, Chip8 cpu, std::vector<char> buf) {

    registers* reg = cpu.get_registers();
    u8* memory = cpu.get_memory();
    u16* stack = cpu.get_stack();

    // print startup info -----
    printf("\n\n -----===== STARTUP INFO =====-----\n");

    printf("\n ----- FILE INFO -----\n");
    printf("\nFile name:\t\t%s", file);
    printf("\nTitle:\t\t\t");
    int i = 0;
    while (buf[i] < 0x41 || buf[i] > 0x5a) { i++; }
    while (buf[i] >= 0x41 && buf[i] <= 0x5a || buf[i] == 0x20) {
        printf("%c", (char)buf[i++]);
    }
    printf("\nSize:\t\t\t%i Bytes / %.2f kB\n", (int)buf.size(), buf.size() / 1000.0);

    printf("\n\n ----- MEMORY MAP -----\n\nAddr.\t");
    for (int i = 0; i < 0x10; i++) {
        printf("0x%.2x ", i);
    }
    printf("\n\t\t\t\t\t\t\t\t\t\t\t ___Interpreter reserved\n");
    int mem_size = cpu.get_mem_size();
    int y;
    for (y = 0; y < mem_size / 0x10; y++) {
        printf("0x%.3x\t", y * 0x10);
        for (int x = 0; x < 0x10; x++) {
            printf("  %.2x ", memory[y * 0x10 + x]);
        }
        if (y == 0x1f) printf(" ___Chip-8 program");
        printf("\n");
    }
    printf("0x%.3x\t", y * 0x10);
    for (int i = 0; i < mem_size % 0x10; i++) {
        printf("  %.2x ", memory[y * 0x10 + i]);
    }
    printf("\n");

    printf("\n\n ----- CPU -----\n\n");
    printf("Stack (%i * 2 Bytes):\t\t", CHIP8_STACK_SIZE);
    for (int x = 0; x < CHIP8_STACK_SIZE; x++) {
        if (x % 8 == 0) printf("\n0x%.2x to 0x%.2x:\t", (x / 8) * 0x8, (x / 8) * 0x8 + 0x7);
        printf("0x%.4x ", stack[x]);
    }
    printf("\n\nRegisters:\n");
    printf("PC:\t\t0x%.4x\t(Program Counter)\n", (*reg).PC);
    printf("SP:\t\t0x%.2x\t(Stack Pointer)\n", (*reg).SP);
    printf("I:\t\t0x%.4x\t(Mainly addresses)\n", (*reg).I);
    printf("dt:\t\t0x%.2x\t(Delay timer at 60Hz)\n", (*reg).dt);
    printf("st:\t\t0x%.2x\t(Sound timer at 60Hz)\n", (*reg).st);
    printf("V (Data registers):");
    for (int x = 0; x < CHIP8_REGISTERS_SIZE; x++) {
        if (x % 8 == 0) printf("\n0x%.2x to 0x%.2x:\t", (x / 8) * 0x8, (x / 8) * 0x8 + 0x7);
        printf("0x%.2x ", (*reg).V[x]);
    }

    std::cout << "\n\n";
}
