#include <oslib/oslib.h>
#include <pspkernel.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspdisplay.h>
#include <pspaudio.h>
#include <pspaudiolib.h>
#include <pspaudio_kernel.h>
#include <psprtc.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#define printf pspDebugScreenPrintf

PSP_MODULE_INFO("Freebox Remote", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(12*1024);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int runningFlag = 1;
char buffer[150] = "";
CURL *curl;
CURLcode res;

int testeur = 0;
void test(){
	testeur = 40;
}

void zap(char* action,char* appui, int codetv){
	test();
	curl_global_cleanup();
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl){
		sprintf(buffer, "http://hd1.freebox.fr/pub/remote_control?key=%s&long=%s&code=%d",action,appui,codetv);
		curl_easy_setopt(curl, CURLOPT_URL, buffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if(CURLE_OK != res) {
			oslDebug("Erreur : %d",res);
		 }
	}
	curl_global_cleanup();
	
	//oslNetPostForm("http://hd1.freebox.fr/pub/remote_control","key=mute&long=false&code=10545415","",255);
	//oslNetGetFile("http://hd1.freebox.fr/pub/remote_control?key=mute&long=false&code=10545415","ms0:/");
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Exit callback */
int exit_callback(int arg1, int arg2, void *common) {
    runningFlag = 0;
    return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp) {
    int cbid;

    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void) {
    int thid = 0;
    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
    if(thid >= 0)
        sceKernelStartThread(thid, 0, 0);
    return thid;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Init OSLib:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int initOSLib(){
    oslInit(0);
    oslInitGfx(OSL_PF_8888, 1);
    oslInitAudio();
	oslInitAudioME(OSL_FMT_MP3);
    oslSetQuitOnLoadFailure(1);
    oslSetKeyAutorepeatInit(40);
    oslSetKeyAutorepeatInterval(10);
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main(){
    int skip = 0;
	char messVersion[100] = "";
	sprintf(messVersion, "Freebox Remote v1.0 - Développé par Miles (mileskabal.com/psp)");
	char messCode[150] = "";
	sprintf(messCode, "Vous devez saisir le code télécommande de votre freebox\nAppuyez sur SELECT pour le rentrer ou l'éditer...");
	char message[100] = "";
	sprintf(message, "Vous devez être connecté pour utiliser l'homebrew\nAppuyer sur START pour vous connecter...");
    int dialog = OSL_DIALOG_NONE;
	//SetupCallbacks();
    initOSLib();
	oslNetInit();
	
	oslIntraFontInit(INTRAFONT_CACHE_ALL | INTRAFONT_STRING_CP1252);
	
	OSL_FONT *pgfFont = oslLoadFontFile("flash0:/font/ltn0.pgf");
    oslIntraFontSetStyle(pgfFont, 0.70, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
	OSL_FONT *pgfFontR = oslLoadFontFile("flash0:/font/ltn0.pgf");
    oslIntraFontSetStyle(pgfFontR, 0.70, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_RIGHT);
	OSL_FONT *pgfFontRN = oslLoadFontFile("flash0:/font/ltn0.pgf");
    oslIntraFontSetStyle(pgfFontRN, 0.50, RGBA(0,0,0,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_RIGHT);
	OSL_FONT *pgfFontB = oslLoadFontFile("flash0:/font/ltn0.pgf");
    oslIntraFontSetStyle(pgfFontB, 0.70, RGBA(0,0,0,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
	OSL_FONT *pgfFontP = oslLoadFontFile("flash0:/font/ltn0.pgf");
    oslIntraFontSetStyle(pgfFontP, 0.50, RGBA(0,0,0,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
    oslSetFont(pgfFont);
	
	OSL_IMAGE *bg, *touches, *touchesL,  *touchesR, *touchesLR;
	bg = oslLoadImageFilePNG ("data/bg.png", OSL_IN_RAM,OSL_PF_8888);
	touches = oslLoadImageFilePNG ("data/touches.png", OSL_IN_RAM,OSL_PF_8888);
	touchesL = oslLoadImageFilePNG ("data/touchesL.png", OSL_IN_RAM,OSL_PF_8888);
	touchesR = oslLoadImageFilePNG ("data/touchesR.png", OSL_IN_RAM,OSL_PF_8888);
	touchesLR = oslLoadImageFilePNG ("data/touchesLR.png", OSL_IN_RAM,OSL_PF_8888);
	
	char statutAppui[6] = "false";
	
	char ligne[500];
	int code=0;
	char codeTxt[20] = "";
	
	FILE *fichierCode = fopen("code.txt","r");
	int k = 0;
	if(fichierCode!=NULL){
		while(fgets(ligne,sizeof(ligne),fichierCode)){
			if(k==0){
			sprintf(codeTxt, ligne);
			code = atoi(ligne); 
			}
			k++;
		}
		fclose(fichierCode);
	}
	
	
	int connected = 0;
	int frame = 0;
	
    while(runningFlag && !osl_quit){
		
        if (!skip){
			
			frame++;
			
			if(oslIsWlanConnected()){
				connected = 1;
			}
			else{
				connected = 0;
			}
			
			oslReadKeys();
			oslStartDrawing();
			oslDrawImageXY(bg,0,0);
			
			if(connected){
				sprintf(messVersion, "Freebox Remote v1.0 - Développé par Miles (mileskabal.free.fr/psp)");
				if (!osl_keys->held.R  && !osl_keys->held.L){ 
				oslDrawImageXY(touches,0,0);
				}
				if (!osl_keys->held.R  && osl_keys->held.L){ 
				oslDrawImageXY(touchesL,0,0);
				}
				if (osl_keys->held.R  && !osl_keys->held.L){ 
				oslDrawImageXY(touchesR,0,0);
				}
				if (osl_keys->held.R  && osl_keys->held.L){ 
				oslDrawImageXY(touchesLR,0,0);
				}
			}
			
			oslSetFont(pgfFontB);
			oslDrawString(5, 160, message);
			oslSetFont(pgfFontRN);
			oslDrawStringf(470, 255, messVersion);
			
			if(connected){
				oslSetFont(pgfFont);
				oslDrawString(5, 50, "Connecté");
				oslSetFont(pgfFontR);
				oslDrawStringf(470, 50, "Code : %d", code);
				
			}
			else{
				dialog = oslGetDialogType();
				if (dialog){
					oslDrawDialog();
					if (oslGetDialogStatus() == PSP_UTILITY_DIALOG_NONE){
						if (oslDialogGetResult() == OSL_DIALOG_CANCEL){
							sprintf(messVersion, "Freebox Remote v1.0 - Développé par Miles (mileskabal.free.fr/psp)");
							sprintf(message, "Vous devez être connecté pour utiliser l'homebrew\nAppuyer sur START pour vous connecter...");
							sprintf(messCode, "Vous devez saisir le code télécommande de votre freebox\nAppuyez sur SELECT pour le rentrer ou l'éditer...");
						}
						oslEndDialog();
					}
				}
				oslSetFont(pgfFontB);
				oslDrawString(5, 100, messCode);
				oslSetFont(pgfFont);
				oslDrawString(5, 50, "Non connecté");
				oslSetFont(pgfFontR);
				oslDrawStringf(470, 50, "Code : %d", code);
			}
			
			if(testeur > 0){
				testeur--;
			}
			
			OSL_COLOR blue = RGB(91, 126, 215);
			int battery = scePowerGetBatteryLifePercent();
			oslSetAlpha(OSL_FX_ALPHA, 100);
			oslDrawRect(446, 2, 468, 12, 0xFF000000);
			oslDrawFillRect(447, 3, 447 + (22* battery / 100), 11, blue); //FF2A770E
			oslDrawRect(467, 4, 470, 10, 0xFF000000);
			oslSetAlpha(OSL_FX_RGBA, 100);
			
			
			if (dialog == OSL_DIALOG_NONE && !connected && !oslOskIsActive()){
				if (osl_keys->pressed.start){
					oslInitNetDialog();
					memset(message, 0, sizeof(message));
					memset(messCode, 0, sizeof(messCode));
					memset(messVersion, 0, sizeof(messVersion));
					
				}
			}
			
			
			if(connected && !testeur){
				if (!osl_keys->held.R  && !osl_keys->held.L){ 
					if (osl_keys->pressed.cross){
						zap("prgm_dec","false",code);
					}
					else if (osl_keys->pressed.triangle){
						zap("prgm_inc","false",code);
					}
					else if (osl_keys->pressed.square){
						zap("vol_dec","false",code);
					}
					else if (osl_keys->pressed.circle){
						zap("vol_inc","false",code);
					}
					else if (osl_keys->pressed.up){
						zap("info","false",code);
					}
					else if (osl_keys->pressed.left){
						zap("mail","false",code);
					}
					else if (osl_keys->pressed.right){
						zap("help","false",code);
					}
					else if (osl_keys->pressed.down){
						zap("ok","false",code);
					}
					if (osl_keys->pressed.start){
						zap("home","false",code);	
					}
					if (osl_keys->pressed.select){
						zap("list","false",code);	
					}
				}
				if (!osl_keys->held.R  && osl_keys->held.L){ 
					if (osl_keys->pressed.cross){
						zap("blue","false",code);
					}
					else if (osl_keys->pressed.triangle){
						zap("red","false",code);
					}
					else if (osl_keys->pressed.square){
						zap("yellow","false",code);
					}
					else if (osl_keys->pressed.circle){
						zap("green","false",code);
					}
					else if (osl_keys->pressed.up){
						zap("info","true",code);
					}
					else if (osl_keys->pressed.left){
						zap("mail","true",code);
					}
					else if (osl_keys->pressed.right){
						zap("help","true",code);
					}
					else if (osl_keys->pressed.down){
						zap("mute","false",code);
					}
					if (osl_keys->pressed.start){
						zap("swap","false",code);	
					}
					if (osl_keys->pressed.select){
						zap("power","false",code);	
					}
				}
				if (osl_keys->held.R  && !osl_keys->held.L){ 
					if (osl_keys->pressed.cross){
						zap("play","false",code);
					}
					else if (osl_keys->pressed.triangle){
						zap("stop","false",code);
					}
					else if (osl_keys->pressed.square){
						zap("prev","false",code);
					}
					else if (osl_keys->pressed.circle){
						zap("next","false",code);
					}
					else if (osl_keys->pressed.up){
						zap("up","false",code);
					}
					else if (osl_keys->pressed.left){
						zap("left","false",code);
					}
					else if (osl_keys->pressed.right){
						zap("right","false",code);
					}
					else if (osl_keys->pressed.down){
						zap("down","false",code);
					}
					if (osl_keys->pressed.start){
						zap("rec","false",code);	
					}
					if (osl_keys->pressed.select){
						zap("back","false",code);	
					}
				}
				if (osl_keys->held.R  && osl_keys->held.L){ 
					if (osl_keys->pressed.cross){
						zap("5",statutAppui,code);
					}
					else if (osl_keys->pressed.triangle){
						zap("6",statutAppui,code);
					}
					else if (osl_keys->pressed.square){
						zap("7",statutAppui,code);
					}
					else if (osl_keys->pressed.circle){
						zap("8",statutAppui,code);
					}
					else if (osl_keys->pressed.up){
						zap("1",statutAppui,code);
					}
					else if (osl_keys->pressed.left){
						zap("2",statutAppui,code);
					}
					else if (osl_keys->pressed.right){
						zap("3",statutAppui,code);
					}
					else if (osl_keys->pressed.down){
						zap("4",statutAppui,code);
					}
					else if (osl_keys->pressed.start){
						zap("0",statutAppui,code);
					}
					else if (osl_keys->pressed.select){
						zap("9",statutAppui,code);
					}
					else if (osl_keys->analogX > 110){
						sprintf(statutAppui, "false");
					}
					else if (osl_keys->analogX < -110){
						sprintf(statutAppui, "true");
					}
				}
				
			}

			
		
			
			if (oslOskIsActive()){
				oslDrawOsk();
				if (oslGetOskStatus() == PSP_UTILITY_DIALOG_NONE){
					if (oslOskGetResult() == OSL_OSK_CANCEL)
						sprintf(messCode, "Vous devez saisir le code télécommande de votre freebox\nAppuyez sur SELECT pour le rentrer ou l'éditer...");
					else{
						char userText[100] = "";
						oslOskGetText(userText);
						FILE *fichierCodes = fopen("code.txt","w+");
						if(fichierCodes!=NULL){
							fputs(userText,fichierCodes);
							sprintf(codeTxt, userText);
							code = atoi(userText); 
							fclose(fichierCodes);
						}
						sprintf(messCode, "Le code %s a bien été enregistré\nAppuyez sur SELECT pour l'éditer...", userText);
						
					}
					oslEndOsk();
				}
			}
			
            oslEndDrawing();
		
		}

		if (!oslOskIsActive() && !connected){
			if (osl_keys->pressed.select){
				oslInitOsk("Inserez le code Freebox", codeTxt, 128, 1,-1);
				memset(messCode, 0, sizeof(messCode));
			}
		}
		
		oslEndFrame();
	    skip = oslSyncFrame();
		
		
    }
	
    //Quit OSL:
	oslEndGfx();
    oslNetTerm();
    oslQuit();
	
    sceKernelExitGame();
    return 0;

}
