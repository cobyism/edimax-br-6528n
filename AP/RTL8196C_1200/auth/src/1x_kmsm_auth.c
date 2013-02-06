#include <stdlib.h>
#include <string.h>

#include "1x_kmsm.h"
#include "1x_info.h"
#include "1x_types.h"
#include "1x_kmsm_eapolkey.h"
#include "1x_ioctl.h"

#include "1x_auth_pae.h"
#include "1x_eapol.h"
#include "1x_radius.h"


#ifdef DBG_WPA_CLIENT
#include "1x_supp_pae.h"
extern Dot1x_Client		RTLClient;
#endif

//#define FOURWAY_DEBUG

#define SOLVE_DUP_4_2
//-------------------------------------------------------------
// Execution of state machine in 802.11i/D3.0 p.113
//-------------------------------------------------------------
int lib1x_akmsm_AssociationRequest( Global_Params * global);
int lib1x_akmsm_AuthenticationRequest( Global_Params * global);
int lib1x_akmsm_AuthenticationSuccess( Global_Params * global);
int lib1x_akmsm_Disconnect( Global_Params * global);
int lib1x_akmsm_EAPOLKeyRecvd( Global_Params * global);
int lib1x_akmsm_IntegrityFailure( Global_Params * global);

//-------------------------------------------------------------
// Procedure called by key management state machine block
//-------------------------------------------------------------
int  lib1x_akmsm_ProcessEAPOL_proc(Global_Params * global);
int  lib1x_akmsm_SendEAPOL_proc(Global_Params * global);
void lib1x_akmsm_Timer_proc(Dot1x_Authenticator * auth);
int  lib1x_akmsm_GroupReKey_Timer_proc(Dot1x_Authenticator * auth);
int  lib1x_akmsm_UpdateGK_proc(Dot1x_Authenticator *auth);

//-------------------------------------------------------------
// Called by external global authenticator
//-------------------------------------------------------------
void lib1x_akmsm_execute( Global_Params * global);



inline void PRINT_GLOBAL_EVENTID(Global_Params * global)
{
	switch( global->EventId )
	{
	case	akmsm_EVENT_AuthenticationRequest:
		printf("%s: EventId = akmsm_EVENT_AuthenticationRequest\n", __FUNCTION__);
		break;
	case	akmsm_EVENT_ReAuthenticationRequest:
		printf("%s: EventId = akmsm_EVENT_ReAuthenticationRequest\n", __FUNCTION__);
		break;

	case    akmsm_EVENT_AuthenticationSuccess:
		printf("%s: EventId = akmsm_EVENT_AuthenticationSuccess\n", __FUNCTION__);
		break;

	case	akmsm_EVENT_Disconnect:
		printf("%s: EventId = akmsm_EVENT_Disconnect\n", __FUNCTION__);
		break;
	case	akmsm_EVENT_DeauthenticationRequest:
		printf("%s: EventId = akmsm_EVENT_DeauthenticationRequest\n", __FUNCTION__);
		break;
	case    akmsm_EVENT_Init:
		printf("%s: EventId = akmsm_EVENT_Init\n", __FUNCTION__);
		break;
	case    akmsm_EVENT_Disassociate:
		printf("%s: EventId = akmsm_EVENT_Disassociate\n", __FUNCTION__);
		break;

	case	akmsm_EVENT_IntegrityFailure:
		printf("%s: EventId = akmsm_EVENT_IntegrityFailure\n", __FUNCTION__);
		break;
	case	akmsm_EVENT_EAPOLKeyRecvd:
		printf("%s: EventId = akmsm_EVENT_EAPOLKeyRecvd\n", __FUNCTION__);
		break;

	case    akmsm_EVENT_TimeOut:
		printf("%s: EventId = akmsm_EVENT_TimeOut\n", __FUNCTION__);
		break;

	default:
		printf("%s: Unknown EventId = %d\n", __FUNCTION__, global->EventId);
		break;
	}//switch
}


#ifndef COMPACK_SIZE
inline void PRINT_GLOBAL_AKM_SM_STATE(Global_Params * global)
{
	switch(global->akm_sm->state)
	{
	case akmsm_AUTHENTICATION2:
    		printf("%s: akm_sm->state = akmsm_AUTHENTICATION2\n", __FUNCTION__);
			break;
	case akmsm_PTKSTART:
    		printf("%s: akm_sm->state = akmsm_PTKSTART\n", __FUNCTION__);
			break;
	case akmsm_PTKINITNEGOTIATING:
    		printf("%s: akm_sm->state = akmsm_PTKINITNEGOTIATING\n", __FUNCTION__);
			break;
	default:
    		printf("%s: akm_sm->state = akmsm_Unknown\n", __FUNCTION__);
			break;
        /*
        case akmsm_PTKSTART:
        case akmsm_PTKINITNEGOTIATING:
        case akmsm_PTKINITDONE:
                global->EventId = akmsm_EVENT_EAPOLKeyRecvd;
                retVal = TRUE;
        */
	}
}
#endif

#ifndef COMPACK_SIZE
inline void PRINT_GLOBAL_AKM_SM_GSTATE(Global_Params * global)
{
	switch(global->akm_sm->gstate)
	{
	case gkmsm_REKEYNEGOTIATING:
    		printf("%s: akm_sm->state = gkmsm_REKEYNEGOTIATING\n", __FUNCTION__);
			break;
	default:
    		printf("%s: akm_sm->state = gkmsm_Unknown\n", __FUNCTION__);
			break;
	}
}
#endif


#ifndef COMPACK_SIZE
inline void PRINT_MAC_ADDRESS(u_char *a, u_char *s)
{
	printf("%s: %02x:%02x:%02x:%02x:%02x:%02x\n", s, a[0], a[1], a[2], a[3], a[4], a[5]);
}
#endif



int lib1x_akmsm_SendEAPOL_proc(Global_Params * global)
{
	APKeyManage_SM	*	akm_sm = global->akm_sm;
	AGKeyManage_SM	*   gkm_sm = global->auth->gk_sm;
	OCTET_STRING	IV, RSC, KeyID, MIC, KeyData;
	lib1x_eapol_key *eapol_key;
	u_short tmpKeyData_Length;

	global->EAPOLMsgSend.Octet = global->theAuthenticator->sendBuffer;
	global->EapolKeyMsgSend.Octet = global->EAPOLMsgSend.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN ;
	eapol_key = (lib1x_eapol_key  * )global->EapolKeyMsgSend.Octet;

	//lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_SendEAPOL_proc\n");

	IV.Octet = (u_char*)malloc(KEY_IV_LEN);
	IV.Length = KEY_IV_LEN;
	RSC.Octet = (u_char*)malloc(KEY_RSC_LEN);
	RSC.Length = KEY_RSC_LEN;
	KeyID.Octet = (u_char*)malloc(KEY_ID_LEN);
	KeyID.Length = KEY_ID_LEN;
	MIC.Octet = (u_char*)malloc(KEY_MIC_LEN);
	MIC.Length = KEY_MIC_LEN;
	KeyData.Octet = (u_char*)malloc(INFO_ELEMENT_SIZE);
	KeyData.Length = 0;

	switch(akm_sm->state)
	{
		case akmsm_PTKSTART:

			//send 1st message of 4-way handshake
#ifdef FOURWAY_DEBUG
			printf("4-1\n");
#endif
			memset(global->EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);
#ifdef RTL_WPA2
#ifdef FOURWAY_DEBUG
			printf("supp_addr = %02X:%02X:%02X:%02X:%02X:%02X\n",
				global->theAuthenticator->supp_addr[0],
				global->theAuthenticator->supp_addr[1],
				global->theAuthenticator->supp_addr[2],
				global->theAuthenticator->supp_addr[3],
				global->theAuthenticator->supp_addr[4],
				global->theAuthenticator->supp_addr[5]);
			printf("4-1: WPA2Enabled = %s\n", global->RSNVariable.WPA2Enabled?"TRUE":"FALSE");
#endif

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
			LOG_MSG_NOTICE("Authenticating......;note:%02x-%02x-%02x-%02x-%02x-%02x;",
				global->theAuthenticator->supp_addr[0],
				global->theAuthenticator->supp_addr[1],
				global->theAuthenticator->supp_addr[2],
				global->theAuthenticator->supp_addr[3],
				global->theAuthenticator->supp_addr[4],
				global->theAuthenticator->supp_addr[5]);
#endif

			if ( global->RSNVariable.WPA2Enabled ) {
				if ( global->RSNVariable.UnicastCipher == DOT11_ENC_CCMP )
					global->KeyDescriptorVer = key_desc_ver2;
				Message_setDescType(global->EapolKeyMsgSend, desc_type_WPA2);
			} else {
				if ( global->RSNVariable.UnicastCipher == DOT11_ENC_CCMP )
					global->KeyDescriptorVer = key_desc_ver2;
				Message_setDescType(global->EapolKeyMsgSend, desc_type_RSN);
			}
#else
			Message_setDescType(global->EapolKeyMsgSend, global->DescriptorType);
#endif
			Message_setKeyDescVer(global->EapolKeyMsgSend, global->KeyDescriptorVer);
			Message_setKeyType(global->EapolKeyMsgSend, type_Pairwise);
			Message_setKeyIndex(global->EapolKeyMsgSend, 0);
			Message_setInstall(global->EapolKeyMsgSend, 0);
			Message_setKeyAck(global->EapolKeyMsgSend, 1);
			Message_setKeyMIC(global->EapolKeyMsgSend, 0);
			Message_setSecure(global->EapolKeyMsgSend, 0);
			Message_setError(global->EapolKeyMsgSend, 0);
			Message_setRequest(global->EapolKeyMsgSend, 0);
			Message_setReserved(global->EapolKeyMsgSend, 0);

			Message_setKeyLength(global->EapolKeyMsgSend, (global->RSNVariable.UnicastCipher  == DOT11_ENC_TKIP) ? 32:16);

#ifdef RTL_WPA2
			// make 4-1's ReplyCounter increased
			Message_setReplayCounter(global->EapolKeyMsgSend, global->akm_sm->CurrentReplayCounter.field.HighPart, global->akm_sm->CurrentReplayCounter.field.LowPart);
			memcpy(&global->akm_sm->ReplayCounterStarted, &global->akm_sm->CurrentReplayCounter, sizeof(LARGE_INTEGER)); // save started reply counter, david+1-11-2007
			INCLargeInteger(&global->akm_sm->CurrentReplayCounter);
#else
			Message_setReplayCounter(global->EapolKeyMsgSend, global->akm_sm->CurrentReplayCounter.field.HighPart, global->akm_sm->CurrentReplayCounter.field.LowPart);
#endif

			INCOctet32_INTEGER(&global->auth->Counter);
#ifndef RTL_WPA2_PREAUTH
			// ANonce is only updated in lib1x_init_authenticator()
			// or after 4-way handshake
			// To avoid different ANonce values among multiple issued 4-1 messages because of multiple association requests
			// Different ANonce values among multiple 4-1 messages induce 4-2 MIC failure.
			SetNonce(global->akm_sm->ANonce, global->auth->Counter);
#endif
			Message_setKeyNonce(global->EapolKeyMsgSend, global->akm_sm->ANonce);

			memset(IV.Octet, 0, IV.Length);
			Message_setKeyIV(global->EapolKeyMsgSend, IV);
			memset(RSC.Octet, 0, RSC.Length);
			Message_setKeyRSC(global->EapolKeyMsgSend, RSC);
			memset(KeyID.Octet, 0, KeyID.Length);
			Message_setKeyID(global->EapolKeyMsgSend, KeyID);
#ifdef RTL_WPA2
			// otherwise PMK cache
			if ( global->RSNVariable.WPA2Enabled && (global->AuthKeyMethod == DOT11_AuthKeyType_RSNPSK || global->RSNVariable.PMKCached) ) {
				static char PMKID_KDE_TYPE[] = { 0xDD, 0x14, 0x00, 0x0F, 0xAC, 0x04 };
				Message_setKeyDataLength(global->EapolKeyMsgSend, 22);
				memcpy(global->EapolKeyMsgSend.Octet + KeyDataPos,
					PMKID_KDE_TYPE, sizeof(PMKID_KDE_TYPE));
				memcpy(global->EapolKeyMsgSend.Octet+KeyDataPos+sizeof(PMKID_KDE_TYPE),
					global->akm_sm->PMKID, PMKID_LEN);
			} else
#endif
			Message_setKeyDataLength(global->EapolKeyMsgSend, 0);

			memset(MIC.Octet, 0, MIC.Length);
			Message_setMIC(global->EapolKeyMsgSend, MIC);

#ifdef RTL_WPA2
			if ( global->RSNVariable.WPA2Enabled ) {
				global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN + Message_KeyDataLength(global->EapolKeyMsgSend);
			} else
#endif
			global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN ;

			global->EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + global->EapolKeyMsgSend.Length;
			break;

		case akmsm_PTKINITNEGOTIATING:
			//send 2nd message of 4-way handshake
			//message has been constructed in the lib1x_akmsm_ProcessEAPOL_proc()

			break;
		case akmsm_PTKINITDONE:
			//send 1st message of 2-way handshake
#ifdef FOURWAY_DEBUG
			printf("2-1\n");
#endif
			memset(global->EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);
#ifdef RTL_WPA2
			if ( global->RSNVariable.WPA2Enabled ) {
				Message_setDescType(global->EapolKeyMsgSend, desc_type_WPA2);
			} else
#endif
				Message_setDescType(global->EapolKeyMsgSend, global->DescriptorType);

			Message_setKeyDescVer(global->EapolKeyMsgSend, global->KeyDescriptorVer);
			Message_setKeyType(global->EapolKeyMsgSend, type_Group);
			Message_setKeyIndex(global->EapolKeyMsgSend, 1);
			Message_setInstall(global->EapolKeyMsgSend, 1);
			Message_setKeyAck(global->EapolKeyMsgSend, 1);
			Message_setKeyMIC(global->EapolKeyMsgSend, 1);
			Message_setSecure(global->EapolKeyMsgSend, 1);
			Message_setError(global->EapolKeyMsgSend, 0);
			Message_setRequest(global->EapolKeyMsgSend, 0);
			Message_setReserved(global->EapolKeyMsgSend, 0);

			global->EapolKeyMsgSend.Octet[1] = 0x03;
// kenny
//			global->EapolKeyMsgSend.Octet[2] = 0x91;
			if(global->KeyDescriptorVer == key_desc_ver1 )
				global->EapolKeyMsgSend.Octet[2] = 0x91;
			else
				global->EapolKeyMsgSend.Octet[2] = 0x92;

			Message_setKeyLength(global->EapolKeyMsgSend, (global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16);

			Message_setReplayCounter(global->EapolKeyMsgSend, global->akm_sm->CurrentReplayCounter.field.HighPart, global->akm_sm->CurrentReplayCounter.field.LowPart);
			INCLargeInteger(&global->akm_sm->CurrentReplayCounter);
			// kenny: n+2
			INCLargeInteger(&global->akm_sm->CurrentReplayCounter);

			SetNonce(global->auth->gk_sm->GNonce, global->auth->Counter);
			Message_setKeyNonce(global->EapolKeyMsgSend, global->auth->gk_sm->GNonce);
			if(global->KeyDescriptorVer == key_desc_ver1 )
				memset(IV.Octet, 0, IV.Length);
			else {
				memset(IV.Octet, 0, IV.Length);
				//memset(IV.Octet, 0xA6, IV.Length);
				//INCOctet32_INTEGER(&global->auth->Counter);
				//SetEAPOL_KEYIV(IV, global->auth->Counter);

			}

			Message_setKeyIV(global->EapolKeyMsgSend, IV);
			lib1x_control_QueryRSC(global, &RSC);
			Message_setKeyRSC(global->EapolKeyMsgSend, RSC);

			memset(KeyID.Octet, 0, KeyID.Length);
			Message_setKeyID(global->EapolKeyMsgSend, KeyID);

#ifdef RTL_WPA2
			if ( global->RSNVariable.WPA2Enabled ) {
				char key_data[128];
				char * key_data_pos = key_data;
				static char GTK_KDE_TYPE[] = { 0xDD, 0x16, 0x00, 0x0F, 0xAC, 0x01, 0x01, 0x00 };
				memcpy(key_data_pos, GTK_KDE_TYPE, sizeof(GTK_KDE_TYPE));
//fix the bug of using default KDE length -----------
				key_data_pos[1] = (unsigned char) 6 + ((global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16);
//------------------------------ david+2006-04-04
				
				key_data_pos += sizeof(GTK_KDE_TYPE);

				global->EapolKeyMsgSend.Octet[1] = 0x13;

				if(global->KeyDescriptorVer == key_desc_ver1)
				{
// david+2006-01-06, fix the bug of using 0 as group key id					
//					global->EapolKeyMsgSend.Octet[2] = 0x81;
					Message_setKeyDescVer(global->EapolKeyMsgSend, key_desc_ver1);					
					Message_setKeyDataLength(global->EapolKeyMsgSend,
						sizeof(GTK_KDE_TYPE) + (((global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16)));
				}else if(global->KeyDescriptorVer == key_desc_ver2)
				{
// david+2006-01-06, fix the bug of using 0 as group key id					
//					global->EapolKeyMsgSend.Octet[2] = 0x82;
					Message_setKeyDescVer(global->EapolKeyMsgSend, key_desc_ver2);
					Message_setKeyDataLength(global->EapolKeyMsgSend,
					    	sizeof(GTK_KDE_TYPE) + ((8 + ((global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16)) ));
				}
				memcpy(key_data_pos, gkm_sm->GTK[gkm_sm->GN], (global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16);


				EncGTK(global, global->akm_sm->PTK + PTK_LEN_EAPOLMIC, PTK_LEN_EAPOLENC,
					key_data,
					sizeof(GTK_KDE_TYPE) + ((global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16),
					 KeyData.Octet, &tmpKeyData_Length);
			} else {
#endif

			if(global->KeyDescriptorVer == key_desc_ver1)
			{
				Message_setKeyDataLength(global->EapolKeyMsgSend,
					((global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16));
			}else if(global->KeyDescriptorVer == key_desc_ver2)
			{
				Message_setKeyDataLength(global->EapolKeyMsgSend,
				    	(8 + ((global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16) ));
			}

			EncGTK(global, global->akm_sm->PTK + PTK_LEN_EAPOLMIC, PTK_LEN_EAPOLENC,
				gkm_sm->GTK[gkm_sm->GN],
				(global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16,
				 KeyData.Octet, &tmpKeyData_Length);
#ifdef RTL_WPA2
			}
#endif
			KeyData.Length = (int)tmpKeyData_Length;
			Message_setKeyData(global->EapolKeyMsgSend, KeyData);

/* Kenny
			global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN +
					((global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16);
*/
			global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN +
					KeyData.Length;

			global->EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN +
					global->EapolKeyMsgSend.Length;


			global->akm_sm->IfCalcMIC = TRUE;
			//sc_yang
			global->akm_sm->TickCnt = SECONDS_TO_TIMERCOUNT(1);
			//sc_yang to count the tick interrupt by timer
			 //LIB_USLEEP(500000);
			//usleep(500000); // for debug

			break;
		default:
			break;

	}//switch

	//lib1x_hexdump2(MESS_DBG_AUTH, "1x_daemon", global->EAPOLMsgSend.Octet, global->EAPOLMsgSend.Length, "Send");
	//KeyDump("lib1x_akmsm_SendEAPOL_proc", global->EAPOLMsgSend.Octet,global->EAPOLMsgSend.Length, "Send EAPOL-KEY");
	//Avaya If the packet is sent first, the TimeoutCtr should be clear, otherwise(if resent), it will not be clear
	akm_sm->TimeoutCtr = 0;
	global->akm_sm->TickCnt = SECONDS_TO_TIMERCOUNT(1);
	global->theAuthenticator->sendhandshakeready = TRUE;

	free(IV.Octet);
	free(RSC.Octet);
	free(KeyID.Octet);
	free(MIC.Octet);
	free(KeyData.Octet);
	return TRUE;
}


int lib1x_akmsm_ProcessEAPOL_proc(Global_Params * global)
/*++
Routine Description:

    Check if the received message is valid. If valid, construct next message
Return Value:
	0	: Suucess. Next sent message is constructed
	not 0 : Fail. Refer to ERROR Message in 1x_kmsm.h

--*/
{
	APKeyManage_SM	*	akm_sm = global->akm_sm;
	int	retVal = 0;
	OCTET_STRING	 IV, RSC, KeyID, MIC;
	lib1x_eapol_key * eapol_key_recvd, * eapol_key_send;
#ifdef RTL_WPA2
	LARGE_INTEGER recievedRC;
	u_short tmpKeyData_Length;
	AGKeyManage_SM	*   	gkm_sm = global->auth->gk_sm;
	OCTET_STRING	 KeyData;
	KeyData.Octet = (u_char*)malloc(INFO_ELEMENT_SIZE);
	KeyData.Length = 0;
#endif


	global->EapolKeyMsgRecvd.Octet = global->EAPOLMsgRecvd.Octet + (ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);
	global->EapolKeyMsgRecvd.Length = global->EAPOLMsgRecvd.Length - (ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);

	global->EapolKeyMsgSend.Octet = global->EAPOLMsgSend.Octet + (ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);
	global->EapolKeyMsgSend.Length = global->EAPOLMsgSend.Length - (ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);

	eapol_key_recvd = (lib1x_eapol_key * )global->EapolKeyMsgRecvd.Octet;
	eapol_key_send = (lib1x_eapol_key * )global->EapolKeyMsgSend.Octet;

	IV.Octet = (u_char*)malloc(KEY_IV_LEN);
	IV.Length = KEY_IV_LEN;
	RSC.Octet = (u_char*)malloc(KEY_RSC_LEN);
	RSC.Length = KEY_RSC_LEN;
	KeyID.Octet = (u_char*)malloc(KEY_ID_LEN);
	KeyID.Length = KEY_ID_LEN;
	MIC.Octet = (u_char*)malloc(KEY_MIC_LEN);
	MIC.Length = KEY_MIC_LEN;




	if(Message_KeyType(global->EapolKeyMsgRecvd) == type_Pairwise)
	{
		switch(akm_sm->state)
		{
			case akmsm_PTKSTART:
			//receive 2nd message and send third
#ifdef FOURWAY_DEBUG
				printf("4-2\n");
#endif
			//check replay counter
#ifdef RTL_WPA2
				Message_ReplayCounter_OC2LI(global->EapolKeyMsgRecvd, &recievedRC);
				INCLargeInteger(&recievedRC);
				if ( !(global->akm_sm->CurrentReplayCounter.field.HighPart == recievedRC.field.HighPart
			             && global->akm_sm->CurrentReplayCounter.field.LowPart == recievedRC.field.LowPart))
#else
				if(!Message_EqualReplayCounter(global->akm_sm->CurrentReplayCounter, global->EapolKeyMsgRecvd))
#endif
				{
#ifdef FOURWAY_DEBUG
					printf("4-2: ERROR_NONEEQUL_REPLAYCOUNTER\n");
#endif
//					syslog(LOG_AUTH|LOG_INFO, "%s: Authentication failled! (4-2: ERROR_NONEEQUL_REPLAYCOUNTER)\n", dev_supp); // david+2006-03-31, add event to syslog
					
					retVal = ERROR_NONEEQUL_REPLAYCOUNTER;
				}else
				{
#ifndef RTL_WPA2
					// kenny: already increase CurrentReplayCounter after 4-1. Do it at the end of 4-2
					INCLargeInteger(&global->akm_sm->CurrentReplayCounter);
#endif
					global->akm_sm->SNonce = Message_KeyNonce(global->EapolKeyMsgRecvd);

					CalcPTK(global->EAPOLMsgRecvd.Octet, global->EAPOLMsgRecvd.Octet + 6,
					global->akm_sm->ANonce.Octet, global->akm_sm->SNonce.Octet,
					global->akm_sm->PMK, PMK_LEN, global->akm_sm->PTK, PTK_LEN_TKIP);

#ifdef DBG_WPA_CLIENT
					{
						memcpy(RTLClient.global->supp_kmsm->PTK, global->akm_sm->PTK, PTK_LEN_TKIP);
						memcpy(RTLClient.global->supp_kmsm->SNonce.Octet, global->akm_sm->SNonce.Octet, KEY_NONCE_LEN);
					}
#endif

					if(!CheckMIC(global->EAPOLMsgRecvd, global->akm_sm->PTK, PTK_LEN_EAPOLMIC))
					{
						global->akm_sm->Disconnect = TRUE;
						global->akm_sm->ErrorRsn = RSN_MIC_failure;
#ifdef RTL_WPA2
						printf("4-2: ERROR_MIC_FAIL\n");

						syslog(LOG_AUTH|LOG_INFO, "%s: Authentication failled! (4-2: MIC error)\n", dev_supp); // david+2006-03-31, add event to syslog

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
						LOG_MSG_NOTICE("Authentication failed;note:%02x-%02x-%02x-%02x-%02x-%02x;",
						global->theAuthenticator->supp_addr[0],
						global->theAuthenticator->supp_addr[1],
						global->theAuthenticator->supp_addr[2],
						global->theAuthenticator->supp_addr[3],
						global->theAuthenticator->supp_addr[4],
						global->theAuthenticator->supp_addr[5]);
#endif			

						wpa2_hexdump("PTK:", global->akm_sm->PTK, PTK_LEN);
						wpa2_hexdump("Message 2:", global->EAPOLMsgRecvd.Octet + 14, global->EAPOLMsgRecvd.Length);

						if (global->RSNVariable.PMKCached ) {
							printf("\n%s:%d del_pmksa due to 4-2 ERROR_MIC_FAIL\n", __FUNCTION__, __LINE__);
							global->RSNVariable.PMKCached = FALSE;
							del_pmksa_by_spa(global->theAuthenticator->supp_addr);
						}
#endif
						retVal = ERROR_MIC_FAIL;
					}else
					{
						//lib1x_control_AssocInfo(global, 0, &global->akm_sm->SuppInfoElement);
						//if(!Message_EqualRSNIE(	Message_KeyData(global->EapolKeyMsgRecvd, Message_ReturnKeyDataLength(global->EapolKeyMsgRecvd)),
						//		global->akm_sm->SuppInfoElement, global->akm_sm->SuppInfoElement.Length))
						if(0)
						{
							global->akm_sm->Disconnect = TRUE;
							global->akm_sm->ErrorRsn = RSN_diff_info_element;
							retVal = ERROR_NONEQUAL_RSNIE;
							printf("4-2: ERROR_NONEQUAL_RSNIE\n");
						}else
						{
							//Construct Message3
#ifdef FOURWAY_DEBUG
							printf("4-3\n");
#endif
							memset(global->EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);
#ifdef RTL_WPA2
							if ( global->RSNVariable.WPA2Enabled ) {
								Message_setDescType(global->EapolKeyMsgSend, desc_type_WPA2);
							} else
								Message_setDescType(global->EapolKeyMsgSend, desc_type_RSN);
#else
							Message_setDescType(global->EapolKeyMsgSend, global->DescriptorType);
#endif
							Message_setKeyDescVer(global->EapolKeyMsgSend, Message_KeyDescVer(global->EapolKeyMsgRecvd));
							Message_setKeyType(global->EapolKeyMsgSend, Message_KeyType(global->EapolKeyMsgRecvd));
							Message_setKeyIndex(global->EapolKeyMsgSend, Message_KeyIndex(global->EapolKeyMsgRecvd));


							//lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_ProcessEAPOL_proc, will install bit set = %d\n", global->RSNVariable.isSuppSupportUnicastCipher ? 1:0);
							//Message_setInstall(global->EapolKeyMsgSend, global->RSNVariable.isSuppSupportUnicastCipher ? 1:0);
							Message_setInstall(global->EapolKeyMsgSend, 1);
							Message_setKeyAck(global->EapolKeyMsgSend, 1);
							Message_setKeyMIC(global->EapolKeyMsgSend, 1);
							Message_setSecure(global->EapolKeyMsgSend, global->RSNVariable.isSuppSupportMulticastCipher ? 0:1);
							Message_setError(global->EapolKeyMsgSend, 0);
							Message_setRequest(global->EapolKeyMsgSend, 0);
							Message_setReserved(global->EapolKeyMsgSend, 0);

							Message_setKeyLength(global->EapolKeyMsgSend, (global->RSNVariable.UnicastCipher  == DOT11_ENC_TKIP) ? 32:16);
							Message_setReplayCounter(global->EapolKeyMsgSend, global->akm_sm->CurrentReplayCounter.field.HighPart, global->akm_sm->CurrentReplayCounter.field.LowPart);
							Message_setKeyNonce(global->EapolKeyMsgSend, global->akm_sm->ANonce);
							memset(IV.Octet, 0, IV.Length);
							Message_setKeyIV(global->EapolKeyMsgSend, IV);


#ifdef RTL_WPA2
							if ( global->RSNVariable.WPA2Enabled ) {
								unsigned char key_data[128];
								unsigned char * key_data_pos = key_data;
								int i;
								unsigned char GTK_KDE_TYPE[] = {0xDD, 0x16, 0x00, 0x0F, 0xAC, 0x01, 0x01, 0x00 };

								global->EapolKeyMsgSend.Octet[1] = 0x13;

								if(global->KeyDescriptorVer == key_desc_ver2 ) {
									INCOctet32_INTEGER(&global->auth->Counter);
									SetEAPOL_KEYIV(IV, global->auth->Counter);
									//memset(IV.Octet, 0x0, IV.Length);
									Message_setKeyIV(global->EapolKeyMsgSend, IV);
								}

								// RSN IE
								//printf("%s: global->auth->RSNVariable.AuthInfoElement.Octet[0] = %02X\n", __FUNCTION__, global->auth->RSNVariable.AuthInfoElement.Octet[0]);
								if (global->auth->RSNVariable.AuthInfoElement.Octet[0] == WPA2_ELEMENT_ID) {
									int len = (unsigned char)global->auth->RSNVariable.AuthInfoElement.Octet[1] + 2;
									memcpy(key_data_pos, global->auth->RSNVariable.AuthInfoElement.Octet, len);
									key_data_pos += len;
								} else {
									//find WPA2_ELEMENT_ID 0x30
									int len = (unsigned char)global->auth->RSNVariable.AuthInfoElement.Octet[1] + 2;
									//printf("%s: global->auth->RSNVariable.AuthInfoElement.Octet[%d] = %02X\n", __FUNCTION__, len, global->auth->RSNVariable.AuthInfoElement.Octet[len]);
									if (global->auth->RSNVariable.AuthInfoElement.Octet[len] == WPA2_ELEMENT_ID) {
										int len2 = (unsigned char)global->auth->RSNVariable.AuthInfoElement.Octet[len+1] + 2;
										memcpy(key_data_pos, global->auth->RSNVariable.AuthInfoElement.Octet+len, len2);
										key_data_pos += len2;
									} else {
										printf("kenny: %s-%d ERROR!\n", __FUNCTION__, __LINE__);
									}
								}


								memcpy(key_data_pos, GTK_KDE_TYPE, sizeof(GTK_KDE_TYPE));
								key_data_pos[1] = (unsigned char) 6 + ((global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16);
								key_data_pos += sizeof(GTK_KDE_TYPE);


								// FIX GROUPKEY ALL ZERO
								global->auth->gk_sm->GInitAKeys = TRUE;
								lib1x_akmsm_UpdateGK_proc(global->auth);
								memcpy(key_data_pos, gkm_sm->GTK[gkm_sm->GN], (global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16);

								key_data_pos += (global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16;
								i = (key_data_pos - key_data) % 8;
								if ( i != 0 ) {
									*key_data_pos = 0xdd;
									key_data_pos++;
									for (i=i+1; i<8; i++) {
										*key_data_pos = 0x0;
										key_data_pos++;
									}

								}

								EncGTK(global, global->akm_sm->PTK + PTK_LEN_EAPOLMIC, PTK_LEN_EAPOLENC,
									key_data,
									(key_data_pos - key_data),
									 KeyData.Octet, &tmpKeyData_Length);

								KeyData.Length = (int)tmpKeyData_Length;
								Message_setKeyData(global->EapolKeyMsgSend, KeyData);
								Message_setKeyDataLength(global->EapolKeyMsgSend, KeyData.Length);

								global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN +
										KeyData.Length;
								lib1x_control_QueryRSC(global, &RSC);
								Message_setKeyRSC(global->EapolKeyMsgSend, RSC);


							} else {
								memset(RSC.Octet, 0, RSC.Length);
								Message_setKeyRSC(global->EapolKeyMsgSend, RSC);
								memset(KeyID.Octet, 0, KeyID.Length);
								Message_setKeyID(global->EapolKeyMsgSend, KeyID);
								//lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_ProcessEAPOL_proc", global->auth->RSNVariable.AuthInfoElement.Octet, global->auth->RSNVariable.AuthInfoElement.Length,"Append Authenticator Information Element");

								{ //WPA 0xDD
									//printf("%s: global->auth->RSNVariable.AuthInfoElement.Octet[0] = %02X\n", __FUNCTION__, global->auth->RSNVariable.AuthInfoElement.Octet[0]);

									int len = (unsigned char)global->auth->RSNVariable.AuthInfoElement.Octet[1] + 2;

									if (global->auth->RSNVariable.AuthInfoElement.Octet[0] == RSN_ELEMENT_ID) {
										memcpy(KeyData.Octet, global->auth->RSNVariable.AuthInfoElement.Octet, len);
										KeyData.Length = len;
									} else {
										// impossible case??
										int len2 = (unsigned char)global->auth->RSNVariable.AuthInfoElement.Octet[len+1] + 2;
										memcpy(KeyData.Octet, global->auth->RSNVariable.AuthInfoElement.Octet+len, len2);
										KeyData.Length = len2;
									}
								}
								Message_setKeyDataLength(global->EapolKeyMsgSend, KeyData.Length);
								Message_setKeyData(global->EapolKeyMsgSend, KeyData);
								global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN + KeyData.Length;
							}

							INCLargeInteger(&global->akm_sm->CurrentReplayCounter);

#else
							memset(RSC.Octet, 0, RSC.Length);
							Message_setKeyRSC(global->EapolKeyMsgSend, RSC);
							memset(KeyID.Octet, 0, KeyID.Length);
							Message_setKeyID(global->EapolKeyMsgSend, KeyID);
							//lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_ProcessEAPOL_proc", global->auth->RSNVariable.AuthInfoElement.Octet, global->auth->RSNVariable.AuthInfoElement.Length,"Append Authenticator Information Element");
							Message_setKeyDataLength(global->EapolKeyMsgSend, global->auth->RSNVariable.AuthInfoElement.Length);
							Message_setKeyData(global->EapolKeyMsgSend, global->auth->RSNVariable.AuthInfoElement);
							//Message_setKeyDataLength(global->EapolKeyMsgSend, global->akm_sm->AuthInfoElement.Length);
							//Message_setKeyData(global->EapolKeyMsgSend, global->akm_sm->AuthInfoElement);
							global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN + global->auth->RSNVariable.AuthInfoElement.Length;
#endif /* RTL_WPA2 */
							global->EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + global->EapolKeyMsgSend.Length;

							global->akm_sm->IfCalcMIC = TRUE;
						}//Message_EqualRSNIE
					}//CheckMIC
				}//Message_EqualReplayCounter
				break;

			case akmsm_PTKINITNEGOTIATING:
			// test 2nd or 4th message

#if 1
// check replay counter to determine if msg 2 or 4 received, david+1-11-2007
//				if ( Message_KeyDataLength(global->EapolKeyMsgRecvd) != 0)
				if(Message_EqualReplayCounter(global->akm_sm->ReplayCounterStarted, global->EapolKeyMsgRecvd))
				{
#ifndef SOLVE_DUP_4_2						
#ifdef FOURWAY_DEBUG
					printf("4-2 in akmsm_PTKINITNEGOTIATING: ERROR_NONEEQUL_REPLAYCOUNTER\n");
#endif
					retVal = ERROR_NONEEQUL_REPLAYCOUNTER;
#else /* SOLVE_DUP_4 */
#ifdef FOURWAY_DEBUG
					printf("4-2 in akmsm_PTKINITNEGOTIATING: resend 4-3\n");
#endif

// copy 4-2 processing from above
				retVal = ERROR_RECV_4WAY_MESSAGE2_AGAIN;
#if 0 // Don't check replay counter during dup 4-2							
#ifdef RTL_WPA2
				Message_ReplayCounter_OC2LI(global->EapolKeyMsgRecvd, &recievedRC);
				INCLargeInteger(&recievedRC);
				if ( !(global->akm_sm->CurrentReplayCounter.field.HighPart == recievedRC.field.HighPart
			             && global->akm_sm->CurrentReplayCounter.field.LowPart == recievedRC.field.LowPart))
#else
				if(!Message_EqualReplayCounter(global->akm_sm->CurrentReplayCounter, global->EapolKeyMsgRecvd))
#endif
				{
#ifdef FOURWAY_DEBUG
					printf("4-2: ERROR_NONEEQUL_REPLAYCOUNTER\n");
					printf("global->akm_sm->CurrentReplayCounter.field.LowPart = %d\n", global->akm_sm->CurrentReplayCounter.field.LowPart);
					printf("recievedRC.field.LowPart = %d\n", recievedRC.field.LowPart);
#endif
					retVal = ERROR_NONEEQUL_REPLAYCOUNTER;
				}else
#endif // Don't check replay counter during dup 4-2				
				{
#ifndef RTL_WPA2
					// kenny: already increase CurrentReplayCounter after 4-1. Do it at the end of 4-2
					INCLargeInteger(&global->akm_sm->CurrentReplayCounter);
#endif
					global->akm_sm->SNonce = Message_KeyNonce(global->EapolKeyMsgRecvd);

					CalcPTK(global->EAPOLMsgRecvd.Octet, global->EAPOLMsgRecvd.Octet + 6,
					global->akm_sm->ANonce.Octet, global->akm_sm->SNonce.Octet,
					global->akm_sm->PMK, PMK_LEN, global->akm_sm->PTK, PTK_LEN_TKIP);

#ifdef DBG_WPA_CLIENT
					{
						memcpy(RTLClient.global->supp_kmsm->PTK, global->akm_sm->PTK, PTK_LEN_TKIP);
						memcpy(RTLClient.global->supp_kmsm->SNonce.Octet, global->akm_sm->SNonce.Octet, KEY_NONCE_LEN);
					}
#endif

					if(!CheckMIC(global->EAPOLMsgRecvd, global->akm_sm->PTK, PTK_LEN_EAPOLMIC))
					{
						global->akm_sm->Disconnect = TRUE;
						global->akm_sm->ErrorRsn = RSN_MIC_failure;
#ifdef RTL_WPA2
						printf("4-2: ERROR_MIC_FAIL\n");

						syslog(LOG_AUTH|LOG_INFO, "%s: Authentication failled! (4-2: MIC error)\n", dev_supp); // david+2006-03-31, add event to syslog

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
						LOG_MSG_NOTICE("Authentication failed;note:%02x-%02x-%02x-%02x-%02x-%02x;",
						global->theAuthenticator->supp_addr[0],
						global->theAuthenticator->supp_addr[1],
						global->theAuthenticator->supp_addr[2],
						global->theAuthenticator->supp_addr[3],
						global->theAuthenticator->supp_addr[4],
						global->theAuthenticator->supp_addr[5]);
#endif

						wpa2_hexdump("PTK:", global->akm_sm->PTK, PTK_LEN);
						wpa2_hexdump("Message 2:", global->EAPOLMsgRecvd.Octet + 14, global->EAPOLMsgRecvd.Length);

						if (global->RSNVariable.PMKCached ) {
							printf("\n%s:%d del_pmksa due to 4-2 ERROR_MIC_FAIL\n", __FUNCTION__, __LINE__);
							global->RSNVariable.PMKCached = FALSE;
							del_pmksa_by_spa(global->theAuthenticator->supp_addr);
						}
#endif
						retVal = ERROR_MIC_FAIL;
					}else
					{
						//lib1x_control_AssocInfo(global, 0, &global->akm_sm->SuppInfoElement);
						//if(!Message_EqualRSNIE(	Message_KeyData(global->EapolKeyMsgRecvd, Message_ReturnKeyDataLength(global->EapolKeyMsgRecvd)),
						//		global->akm_sm->SuppInfoElement, global->akm_sm->SuppInfoElement.Length))
						if(0)
						{
							global->akm_sm->Disconnect = TRUE;
							global->akm_sm->ErrorRsn = RSN_diff_info_element;
							retVal = ERROR_NONEQUAL_RSNIE;
							printf("4-2: ERROR_NONEQUAL_RSNIE\n");
						}else
						{
							//Construct Message3
#ifdef FOURWAY_DEBUG
							printf("4-3\n");
#endif
							memset(global->EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);
#ifdef RTL_WPA2
							if ( global->RSNVariable.WPA2Enabled ) {
								Message_setDescType(global->EapolKeyMsgSend, desc_type_WPA2);
							} else
								Message_setDescType(global->EapolKeyMsgSend, desc_type_RSN);
#else
							Message_setDescType(global->EapolKeyMsgSend, global->DescriptorType);
#endif
							Message_setKeyDescVer(global->EapolKeyMsgSend, Message_KeyDescVer(global->EapolKeyMsgRecvd));
							Message_setKeyType(global->EapolKeyMsgSend, Message_KeyType(global->EapolKeyMsgRecvd));
							Message_setKeyIndex(global->EapolKeyMsgSend, Message_KeyIndex(global->EapolKeyMsgRecvd));


							//lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_ProcessEAPOL_proc, will install bit set = %d\n", global->RSNVariable.isSuppSupportUnicastCipher ? 1:0);
							//Message_setInstall(global->EapolKeyMsgSend, global->RSNVariable.isSuppSupportUnicastCipher ? 1:0);
							Message_setInstall(global->EapolKeyMsgSend, 1);
							Message_setKeyAck(global->EapolKeyMsgSend, 1);
							Message_setKeyMIC(global->EapolKeyMsgSend, 1);
							Message_setSecure(global->EapolKeyMsgSend, global->RSNVariable.isSuppSupportMulticastCipher ? 0:1);
							Message_setError(global->EapolKeyMsgSend, 0);
							Message_setRequest(global->EapolKeyMsgSend, 0);
							Message_setReserved(global->EapolKeyMsgSend, 0);

							Message_setKeyLength(global->EapolKeyMsgSend, (global->RSNVariable.UnicastCipher  == DOT11_ENC_TKIP) ? 32:16);
							Message_setReplayCounter(global->EapolKeyMsgSend, global->akm_sm->CurrentReplayCounter.field.HighPart, global->akm_sm->CurrentReplayCounter.field.LowPart);
							Message_setKeyNonce(global->EapolKeyMsgSend, global->akm_sm->ANonce);
							memset(IV.Octet, 0, IV.Length);
							Message_setKeyIV(global->EapolKeyMsgSend, IV);


#ifdef RTL_WPA2
							if ( global->RSNVariable.WPA2Enabled ) {
								unsigned char key_data[128];
								unsigned char * key_data_pos = key_data;
								int i;
								unsigned char GTK_KDE_TYPE[] = {0xDD, 0x16, 0x00, 0x0F, 0xAC, 0x01, 0x01, 0x00 };

								global->EapolKeyMsgSend.Octet[1] = 0x13;

								if(global->KeyDescriptorVer == key_desc_ver2 ) {
									INCOctet32_INTEGER(&global->auth->Counter);
									SetEAPOL_KEYIV(IV, global->auth->Counter);
									//memset(IV.Octet, 0x0, IV.Length);
									Message_setKeyIV(global->EapolKeyMsgSend, IV);
								}

								// RSN IE
								//printf("%s: global->auth->RSNVariable.AuthInfoElement.Octet[0] = %02X\n", __FUNCTION__, global->auth->RSNVariable.AuthInfoElement.Octet[0]);
								if (global->auth->RSNVariable.AuthInfoElement.Octet[0] == WPA2_ELEMENT_ID) {
									int len = (unsigned char)global->auth->RSNVariable.AuthInfoElement.Octet[1] + 2;
									memcpy(key_data_pos, global->auth->RSNVariable.AuthInfoElement.Octet, len);
									key_data_pos += len;
								} else {
									//find WPA2_ELEMENT_ID 0x30
									int len = (unsigned char)global->auth->RSNVariable.AuthInfoElement.Octet[1] + 2;
									//printf("%s: global->auth->RSNVariable.AuthInfoElement.Octet[%d] = %02X\n", __FUNCTION__, len, global->auth->RSNVariable.AuthInfoElement.Octet[len]);
									if (global->auth->RSNVariable.AuthInfoElement.Octet[len] == WPA2_ELEMENT_ID) {
										int len2 = (unsigned char)global->auth->RSNVariable.AuthInfoElement.Octet[len+1] + 2;
										memcpy(key_data_pos, global->auth->RSNVariable.AuthInfoElement.Octet+len, len2);
										key_data_pos += len2;
									} else {
										printf("kenny: %s-%d ERROR!\n", __FUNCTION__, __LINE__);
									}
								}


								memcpy(key_data_pos, GTK_KDE_TYPE, sizeof(GTK_KDE_TYPE));
								key_data_pos[1] = (unsigned char) 6 + ((global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16);
								key_data_pos += sizeof(GTK_KDE_TYPE);


								// FIX GROUPKEY ALL ZERO
								global->auth->gk_sm->GInitAKeys = TRUE;
								lib1x_akmsm_UpdateGK_proc(global->auth);
								memcpy(key_data_pos, gkm_sm->GTK[gkm_sm->GN], (global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16);

								key_data_pos += (global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16;
								i = (key_data_pos - key_data) % 8;
								if ( i != 0 ) {
									*key_data_pos = 0xdd;
									key_data_pos++;
									for (i=i+1; i<8; i++) {
										*key_data_pos = 0x0;
										key_data_pos++;
									}

								}

								EncGTK(global, global->akm_sm->PTK + PTK_LEN_EAPOLMIC, PTK_LEN_EAPOLENC,
									key_data,
									(key_data_pos - key_data),
									 KeyData.Octet, &tmpKeyData_Length);

								KeyData.Length = (int)tmpKeyData_Length;
								Message_setKeyData(global->EapolKeyMsgSend, KeyData);
								Message_setKeyDataLength(global->EapolKeyMsgSend, KeyData.Length);

								global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN +
										KeyData.Length;
								lib1x_control_QueryRSC(global, &RSC);
								Message_setKeyRSC(global->EapolKeyMsgSend, RSC);


							} else {
								memset(RSC.Octet, 0, RSC.Length);
								Message_setKeyRSC(global->EapolKeyMsgSend, RSC);
								memset(KeyID.Octet, 0, KeyID.Length);
								Message_setKeyID(global->EapolKeyMsgSend, KeyID);
								//lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_ProcessEAPOL_proc", global->auth->RSNVariable.AuthInfoElement.Octet, global->auth->RSNVariable.AuthInfoElement.Length,"Append Authenticator Information Element");

								{ //WPA 0xDD
									//printf("%s: global->auth->RSNVariable.AuthInfoElement.Octet[0] = %02X\n", __FUNCTION__, global->auth->RSNVariable.AuthInfoElement.Octet[0]);

									int len = (unsigned char)global->auth->RSNVariable.AuthInfoElement.Octet[1] + 2;

									if (global->auth->RSNVariable.AuthInfoElement.Octet[0] == RSN_ELEMENT_ID) {
										memcpy(KeyData.Octet, global->auth->RSNVariable.AuthInfoElement.Octet, len);
										KeyData.Length = len;
									} else {
										// impossible case??
										int len2 = (unsigned char)global->auth->RSNVariable.AuthInfoElement.Octet[len+1] + 2;
										memcpy(KeyData.Octet, global->auth->RSNVariable.AuthInfoElement.Octet+len, len2);
										KeyData.Length = len2;
									}
								}
								Message_setKeyDataLength(global->EapolKeyMsgSend, KeyData.Length);
								Message_setKeyData(global->EapolKeyMsgSend, KeyData);
								global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN + KeyData.Length;
							}

							INCLargeInteger(&global->akm_sm->CurrentReplayCounter);

#else
							memset(RSC.Octet, 0, RSC.Length);
							Message_setKeyRSC(global->EapolKeyMsgSend, RSC);
							memset(KeyID.Octet, 0, KeyID.Length);
							Message_setKeyID(global->EapolKeyMsgSend, KeyID);
							//lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_ProcessEAPOL_proc", global->auth->RSNVariable.AuthInfoElement.Octet, global->auth->RSNVariable.AuthInfoElement.Length,"Append Authenticator Information Element");
							Message_setKeyDataLength(global->EapolKeyMsgSend, global->auth->RSNVariable.AuthInfoElement.Length);
							Message_setKeyData(global->EapolKeyMsgSend, global->auth->RSNVariable.AuthInfoElement);
							//Message_setKeyDataLength(global->EapolKeyMsgSend, global->akm_sm->AuthInfoElement.Length);
							//Message_setKeyData(global->EapolKeyMsgSend, global->akm_sm->AuthInfoElement);
							global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN + global->auth->RSNVariable.AuthInfoElement.Length;
#endif /* RTL_WPA2 */
							global->EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + global->EapolKeyMsgSend.Length;

							global->akm_sm->IfCalcMIC = TRUE;
						}//Message_EqualRSNIE
					}//CheckMIC
				}//Message_EqualReplayCounter
				break;
					
#endif /* SOLVE_DUP_4_2 */					
				}else if(!CheckMIC(global->EAPOLMsgRecvd, global->akm_sm->PTK, PTK_LEN_EAPOLMIC))
#else
				if(!CheckMIC(global->EAPOLMsgRecvd, global->akm_sm->PTK, PTK_LEN_EAPOLMIC))

#endif
				{
					global->akm_sm->Disconnect = TRUE;
                                        global->akm_sm->ErrorRsn = RSN_MIC_failure;
					retVal = ERROR_MIC_FAIL;
#ifdef RTL_WPA2
					printf("4-4: RSN_MIC_failure\n");

					syslog(LOG_AUTH|LOG_INFO, "%s: Authentication failled! (4-4: RSN MIC error)\n", dev_supp); // david+2006-03-31, add event to syslog

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
					LOG_MSG_NOTICE("Authentication failed;note:%02x-%02x-%02x-%02x-%02x-%02x;",
						global->theAuthenticator->supp_addr[0],
						global->theAuthenticator->supp_addr[1],
						global->theAuthenticator->supp_addr[2],
						global->theAuthenticator->supp_addr[3],
						global->theAuthenticator->supp_addr[4],
						global->theAuthenticator->supp_addr[5]);
#endif

					if (global->RSNVariable.PMKCached ) {
						printf("\n%s:%d del_pmksa due to 4-4 RSN_MIC_failure\n", __FUNCTION__, __LINE__);
						global->RSNVariable.PMKCached = FALSE;
						del_pmksa_by_spa(global->theAuthenticator->supp_addr);
					}
#endif

				}else
				{
#ifdef FOURWAY_DEBUG
					printf("4-4\n");
#endif
#ifdef RTL_WPA2_PREAUTH
					// update ANonce for next 4-way handshake
					SetNonce(akm_sm->ANonce, global->auth->Counter);
#endif
					syslog(LOG_AUTH|LOG_INFO, "%s: Open and authenticated\n", dev_supp); // david+2006-03-31, add event to syslog

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
					LOG_MSG_NOTICE("Authentication Success;note:%02x-%02x-%02x-%02x-%02x-%02x;",
						global->theAuthenticator->supp_addr[0],
						global->theAuthenticator->supp_addr[1],
						global->theAuthenticator->supp_addr[2],
						global->theAuthenticator->supp_addr[3],
						global->theAuthenticator->supp_addr[4],
						global->theAuthenticator->supp_addr[5]);
#endif

					//MLME-SETKEYS.request
					INCLargeInteger(&global->akm_sm->CurrentReplayCounter);
					// kenny: n+2
					INCLargeInteger(&global->akm_sm->CurrentReplayCounter);
				}
				break;
			case akmsm_PTKINITDONE:

				//receive message [with request bit set]
				if(Message_Request(global->EapolKeyMsgRecvd))
				//supp request to initiate 4-way handshake
				{

				}

				break;
			default:
				printf("%s: akm_sm->state = Unknown.\n", __FUNCTION__);
				break;
		}//switch
	}else if(Message_KeyType(global->EapolKeyMsgRecvd) == type_Group)
	{
#ifdef FOURWAY_DEBUG
		printf("2-2\n");
#endif
		if(!Message_Request(global->EapolKeyMsgRecvd))
		//2nd message of 2-way handshake
		{
			//verify that replay counter maches one it has used in the Group Key handshake

			if(Message_LargerReplayCounter(global->akm_sm->CurrentReplayCounter, global->EapolKeyMsgRecvd))
			{
				retVal = ERROR_LARGER_REPLAYCOUNTER;
			}else if(!CheckMIC(global->EAPOLMsgRecvd, global->akm_sm->PTK, PTK_LEN_EAPOLMIC))
			{
				global->akm_sm->Disconnect = TRUE;
				global->akm_sm->ErrorRsn = RSN_MIC_failure;
				retVal = ERROR_MIC_FAIL;
			}else
			{
				//complete one supplicant group key update
				retVal = 0;
			}

		}else //if(!Message_Request(global->EapolKeyMsgRecvd))
		//supp request to change group key
		{
			printf("%s: Message_Request(global->EapolKeyMsgRecvd).\n", __FUNCTION__);
		}
	}

	free(IV.Octet);
	free(RSC.Octet);
	free(KeyID.Octet);
	free(MIC.Octet);
#ifdef RTL_WPA2
	free(KeyData.Octet);
#endif
	return retVal;
};

#ifndef COMPACK_SIZE
//process 2nd message sent from supplicant and generate 3rd message sent to supplicant
int lib1x_akmsm_PTKSTART_proc(Global_Params * global)
{
	lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_PTKSTART_proc\n");
	return TRUE;
}

//process 4th message from supplicant
int lib1x_akmsm_PTKINITNEGOTIAONT_proc(Global_Params * global)
{
	lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_PTKINITNEGOTIAONT_proc\n");
	return TRUE;
}
#endif
//-------------------------------------------------------------
// Start 2-way handshake after receiving 4th message
// Return 1 success
//-------------------------------------------------------------
int lib1x_akmsm_UpdateGK_proc(Dot1x_Authenticator *auth)
{
	Global_Params *		pGlobal;
	AGKeyManage_SM *	gkm_sm = auth->gk_sm;


	int i, retVal = TRUE;

	lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_UpdateGK_proc\n");


	//------------------------------------------------------------
        // Execute Global Group key state machine
        //------------------------------------------------------------
	if( gkm_sm->GTKAuthenticator &&
		(gkm_sm->GTKRekey || (gkm_sm->GInitAKeys && !gkm_sm->GInitDone)) )
	{
		lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_UpdateGK_proc, New Group Key Will generated\n");
		if(gkm_sm->GTKRekey)
			lib1x_message(MESS_DBG_KEY_MANAGE, "gkm_sm->GTKRekey = TRUE\n");
		if(gkm_sm->GInitAKeys)
			lib1x_message(MESS_DBG_KEY_MANAGE, "gkm_sm->GInitAKeys = TRUE\n");
		if(!gkm_sm->GInitDone)
			lib1x_message(MESS_DBG_KEY_MANAGE, "!gkm_sm->GInitDone = TRUE\n");


		gkm_sm->GTKRekey = FALSE;
		if(!gkm_sm->GInitDone)
		{
			lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_UpdateGK_proc, Group key is first generated\n");
			for(i = 0; i < NumGroupKey; i++)
				memset(gkm_sm->GTK[i], 0, sizeof(gkm_sm->GTK[i]));
			gkm_sm->GN = 1;
			gkm_sm->GM = 2;

		}
		//---- If 0607
		//---- Empty
		//---- Else
		/*else
			SWAP(gkm_sm->GN, gkm_sm->GM);
		*/
		//---- Endif

		gkm_sm->GInitDone = TRUE;

		INCOctet32_INTEGER(&auth->Counter);

		// kenny:??? GNonce should be a random number ???
		SetNonce(gkm_sm->GNonce , auth->Counter);
		CalcGTK(auth->CurrentAddress, gkm_sm->GNonce.Octet,
				gkm_sm->GMK, GMK_LEN, gkm_sm->GTK[gkm_sm->GN], GTK_LEN);
		gkm_sm->GUpdateStationKeys = TRUE;
		gkm_sm->GkeyReady = FALSE;

		//---- In the case of updating GK to all STAs, only the STA that has finished
		//---- 4-way handshake is needed to be sent with 2-way handshake
		//gkm_sm->GKeyDoneStations = auth->NumOfSupplicant;
		gkm_sm->GKeyDoneStations = 0;
		//sc_yang
		for(i = 0 ; i < auth->MaxSupplicant ; i++)
// reduce pre-alloc memory size, david+2006-02-06			
//			if(auth->Supp[i]->global->akm_sm->state == akmsm_PTKINITDONE &&
			if(auth->Supp[i] && auth->Supp[i]->global->akm_sm->state == akmsm_PTKINITDONE &&
				auth->Supp[i]->isEnable)
				gkm_sm->GKeyDoneStations++;
		lib1x_message(MESS_DBG_KEY_MANAGE, "GKeyDoneStations : Number of stations left to have their Group key updated = %d\n", gkm_sm->GKeyDoneStations);
	}


	//------------------------------------------------------------
	// Execute Group key state machine of each STA
	//------------------------------------------------------------
	lib1x_message(MESS_DBG_KEY_MANAGE, "Execute Group key state machine of each STA");
	//sc_yang
	for(i = 0 ; i < auth->MaxSupplicant ; i++)
        {
// reduce pre-alloc memory size, david+2006-02-06			        
//		if(!auth->Supp[i]->isEnable)
		if(auth->Supp[i]==NULL || !auth->Supp[i]->isEnable)
			continue;
		pGlobal = auth->Supp[i]->global;

		//----if 0706
		lib1x_message(MESS_DBG_KEY_MANAGE, "Supp[%d] is enable, update group key\n", i);
		//----else
		//lib1x_message(MESS_DBG_KEY_MANAGE, "Supp[i] is enable, update group key\n", i);
		//----endif


		//---- Group key handshake to only one supplicant ----
		if(pGlobal->akm_sm->state == akmsm_PTKINITDONE &&
			(gkm_sm->GkeyReady && pGlobal->akm_sm->PInitAKeys))
		{
			lib1x_message(MESS_DBG_KEY_MANAGE, "Group Key Update One : STA[%d]\n", auth->Supp[i]->index);
			pGlobal->akm_sm->PInitAKeys = FALSE;
			pGlobal->akm_sm->gstate = gkmsm_REKEYNEGOTIATING;	// set proper gstat, david+2006-04-06
                        lib1x_akmsm_SendEAPOL_proc(pGlobal);

		}
		//---- Updata group key to all supplicant----
		else if(pGlobal->akm_sm->state == akmsm_PTKINITDONE &&           //Done 4-way handshake
                      (gkm_sm->GUpdateStationKeys      ||                     //When new key is generated
                       pGlobal->akm_sm->gstate == gkmsm_REKEYNEGOTIATING))  //1st message is not yet sent
                {
                        lib1x_message(MESS_DBG_KEY_MANAGE, "Group KEY Update ALL : STA[%d]\n", auth->Supp[i]->index);
			pGlobal->akm_sm->PInitAKeys = FALSE;
			pGlobal->akm_sm->gstate = gkmsm_REKEYNEGOTIATING;	// set proper gstat, david+2006-04-06			
			lib1x_akmsm_SendEAPOL_proc(pGlobal);
                }

       }
       gkm_sm->GUpdateStationKeys = FALSE;

	return retVal;
};


void lib1x_akmsm_Timer_proc(Dot1x_Authenticator * auth)
{
	int 			i;
	Global_Params *		global;
	APKeyManage_SM  *       akm_sm;


	/*
	if(lib1x_global_signal_info != NULL)
		auth = (Dot1x_Authenticator *) lib1x_global_signal_info;
	else
		return 0;
	*/


	//sc_yang
	for(i = 0 ; i < auth->MaxSupplicant ; i++)
        {
// reduce pre-alloc memory size, david+2006-02-06        
//		if(!auth->Supp[i]->isEnable)
		if(auth->Supp[i]==NULL || !auth->Supp[i]->isEnable)
			continue;

		global = auth->Supp[i]->global;
		akm_sm = global->akm_sm;

		//lib1x_akmsm_dump(global);


		if(akm_sm->state == akmsm_PTKSTART ||
		   akm_sm->state == akmsm_PTKINITNEGOTIATING ||
		   ( akm_sm->state == akmsm_PTKINITDONE && akm_sm->gstate == gkmsm_REKEYNEGOTIATING) ||
		   akm_sm->bWaitForPacket)
		{
			//sc_yang
			if( --akm_sm->TickCnt == 0){
				akm_sm->TickCnt = SECONDS_TO_TIMERCOUNT(1);
			akm_sm->TimeoutCtr++;
			}
			else
				continue;

#ifdef ALLOW_DBG_KEY_MANAGE
			if(akm_sm->state == akmsm_PTKSTART) printf("akm_sm->state == akmsm_PTKSTART\n");
			if(akm_sm->state == akmsm_PTKINITNEGOTIATING) printf("akm_sm->state == akmsm_PTKINITNEGOTIATING\n");
			if(akm_sm->state == akmsm_PTKINITDONE && akm_sm->gstate == gkmsm_REKEYNEGOTIATING)			printf("akm_sm->state == akmsm_PTKINITDONE && akm_sm->gstate == gkmsm_REKEYNEGOTIATING\n");
			if(akm_sm->bWaitForPacket) printf("akm_sm->bWaitForPacket\n");
#endif

			//lib1x_message(MESS_DBG_KEY_MANAGE, "Supplicant [%d] Timeout Counter = %d", global->index, akm_sm->TimeoutCtr);
			if((akm_sm->state == akmsm_PTKSTART || akm_sm->state == akmsm_PTKINITNEGOTIATING) &&
			    akm_sm->TimeoutCtr > 0)
			    //akm_sm->TimeoutCtr == global->Dot11RSNConfig.PairwiseUpdateCount
			//---- Pairwise Key state machine time out ----
			{


				lib1x_message(MESS_DBG_KEY_MANAGE, "akm_sm->TimeoutCtr = %d, global->Dot11RSNConfig.PairwiseUpdateCount = %d\n", akm_sm->TimeoutCtr, global->Dot11RSNConfig.PairwiseUpdateCount);
				if(akm_sm->TimeoutCtr <= global->Dot11RSNConfig.PairwiseUpdateCount)
				{
					//----Resent packet in buffer
					lib1x_message(MESS_DBG_KEY_MANAGE, "[****Pairwise Key state machine time out], Re-sent Packet\n");
					lib1x_PrintAddr(global->theAuthenticator->supp_addr);
					akm_sm->TimeoutEvt = TRUE;
					
// increase replay counter ----------------------
#if 0
					Message_setReplayCounter(global->EapolKeyMsgSend, global->akm_sm->CurrentReplayCounter.field.HighPart, global->akm_sm->CurrentReplayCounter.field.LowPart);
		                        INCLargeInteger(&global->akm_sm->CurrentReplayCounter);
					global->akm_sm->IfCalcMIC = TRUE;					
#endif					
//----------------------------- david, 2006-08-09	
				}
				else
				{
					//----Clear Timeout Counter, stop send packet
					lib1x_PrintAddr(global->theAuthenticator->supp_addr);
					lib1x_message(MESS_DBG_KEY_MANAGE, "[****Pairwise Key state machine time out], Maxmum Retry time\n");
					akm_sm->TimeoutCtr = 0;
					//global->akm_sm->ErrorRsn = disas_lv_ss;
					global->akm_sm->ErrorRsn = RSN_4_way_handshake_timeout;
					global->akm_sm->Disconnect = TRUE;
#ifdef RTL_WPA2
					if (global->RSNVariable.PMKCached && (akm_sm->state == akmsm_PTKSTART || akm_sm->state == akmsm_PTKINITNEGOTIATING) ) {
						printf("\n%s:%d del_pmksa due to 4-1 or 4-3 timeout\n", __FUNCTION__, __LINE__);
						global->RSNVariable.PMKCached = FALSE;
						del_pmksa_by_spa(global->theAuthenticator->supp_addr);
					}
#endif
				}
			}
			//---- Group Key state machine time out ----


			if(
			    (akm_sm->state == akmsm_PTKINITDONE && akm_sm->gstate == gkmsm_REKEYNEGOTIATING) &&
			     akm_sm->TimeoutCtr > 0)
			     //akm_sm->TimeoutCtr == global->Dot11RSNConfig.GroupUpdateCount

			{
				//The Authenticator must increment and use a new Replay Counter value in every Message 1 instance

				lib1x_message(MESS_DBG_KEY_MANAGE, "akm_sm->TimeoutCtr = %d, global->Dot11RSNConfig.GroupUpdateCount = %d\n", akm_sm->TimeoutCtr, global->Dot11RSNConfig.GroupUpdateCount);


				if(akm_sm->TimeoutCtr <= global->Dot11RSNConfig.GroupUpdateCount)
				{

					lib1x_message(MESS_DBG_KEY_MANAGE, "[*****Group Key state machine time out], Resent Packet\n");
					lib1x_PrintAddr(global->theAuthenticator->supp_addr);
					//07-15
					Message_setReplayCounter(global->EapolKeyMsgSend, global->akm_sm->CurrentReplayCounter.field.HighPart, global->akm_sm->CurrentReplayCounter.field.LowPart);
		                        INCLargeInteger(&global->akm_sm->CurrentReplayCounter);

					//sc_yang n+2
		                        INCLargeInteger(&global->akm_sm->CurrentReplayCounter);

					//
					akm_sm->TimeoutEvt = TRUE;
					global->akm_sm->IfCalcMIC = TRUE;
				}
				else
				{
					lib1x_message(MESS_DBG_KEY_MANAGE, "[*****Group Key state machine time out], Maximun Retry time\n");
					lib1x_PrintAddr(global->theAuthenticator->supp_addr);
					akm_sm->TimeoutCtr = 0;
					global->akm_sm->ErrorRsn = disas_lv_ss;
					global->akm_sm->Disconnect = TRUE;

// set group key to driver and reset rekey timer when GKeyDoneStations=0 -------
					if(global->auth->gk_sm->GKeyDoneStations > 0)
						global->auth->gk_sm->GKeyDoneStations--;

					if (global->auth->gk_sm->GKeyDoneStations == 0 && !global->auth->gk_sm->GkeyReady)
	                {
                	        if(lib1x_control_SetGTK(global) == 0)//success
							{
								printf("last node of group key unpdate expired!\n");
           	            		global->auth->gk_sm->GkeyReady = TRUE;
								global->auth->gk_sm->GResetCounter = TRUE;
							}
	                }		
//--------------------------------------------------------- david+2006-04-06					
				}
			}

		}

	}

}


//--------------------------------------------------------------------
// Return 1 for success
//--------------------------------------------------------------------
int lib1x_akmsm_GroupReKey_Timer_proc(Dot1x_Authenticator * auth)
{

	Global_Params * global;
	Auth_Pae      * auth_pae;


        struct lib1x_eapol 	* eapol;
        struct lib1x_ethernet 	* eth_hdr;


	int i, retVal = TRUE;
	lib1x_message(MESS_DBG_KEY_MANAGE, "=================================");
	lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_GroupReKey_Timer_proc");
	lib1x_message(MESS_DBG_KEY_MANAGE, "=================================");
	auth->gk_sm->GTKRekey = TRUE;
	lib1x_akmsm_UpdateGK_proc(auth);

	//sc_yang
	for(i = 0 ; i < auth->MaxSupplicant ; i++)
        {
// reduce pre-alloc memory size, david+2006-02-06        
//		if(!auth->Supp[i]->isEnable)
		if(auth->Supp[i]==NULL || !auth->Supp[i]->isEnable)
			continue;

		global = auth->Supp[i]->global;
		auth_pae = global->theAuthenticator;

		if( auth_pae->sendhandshakeready )
		{
			//ethernet and eapol header initialization
			eth_hdr = ( struct lib1x_ethernet * )global->EAPOLMsgSend.Octet;
			memcpy ( eth_hdr->ether_dhost , auth_pae->supp_addr, ETHER_ADDRLEN );
			memcpy ( eth_hdr->ether_shost , auth_pae->global->TxRx->oursupp_addr, ETHER_ADDRLEN );
			eth_hdr->ether_type = htons(LIB1X_ETHER_EAPOL_TYPE);

			eapol = ( struct lib1x_eapol * )  ( global->EAPOLMsgSend.Octet +  ETHER_HDRLEN )  ;
			eapol->protocol_version = LIB1X_EAPOL_VER;
			eapol->packet_type =  LIB1X_EAPOL_KEY;

			eapol->packet_body_length = htons(global->EapolKeyMsgSend.Length);

			//lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_execute", auth_pae->sendBuffer, global->EAPOLMsgSend.Length, "Send EAPOL-KEY Message");

			if(global->akm_sm->IfCalcMIC)
			{
				//lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_execute",global->akm_sm->PTK, PTK_LEN_EAPOLMIC, "Key for calculate MIC");
    				CalcMIC(global->EAPOLMsgSend, global->KeyDescriptorVer, global->akm_sm->PTK, PTK_LEN_EAPOLMIC);
				global->akm_sm->IfCalcMIC = FALSE;
			}
			global->akm_sm->TimeoutEvt = 0;
			//global->akm_sm->TimeoutCtr = 0;

			KeyDump("lib1x_akmsm_SendEAPOL_proc", global->EAPOLMsgSend.Octet,global->EAPOLMsgSend.Length, "Send EAPOL-KEY");
			lib1x_nal_send( auth_pae->global->TxRx->network_supp, auth_pae->sendBuffer,
	global->EAPOLMsgSend.Length );
			global->akm_sm->bWaitForPacket = TRUE;
			auth_pae->sendhandshakeready = FALSE;
		}


	}

	return retVal;

}


void lib1x_akmsm_EAPOLStart_Timer_proc(Dot1x_Authenticator * auth)
{
	int	i;
	Global_Params *		global;
	APKeyManage_SM  *       akm_sm;
	Auth_Pae 	       * auth_pae;


	for(i = 0 ; i < auth->MaxSupplicant /*&& auth->Supp[i]->isEnable*/ ; i++)
	{
	// reduce pre-alloc memory size, david+2006-02-06        
		if (auth->Supp[i]==NULL)
			continue;

		global = auth->Supp[i]->global;
		akm_sm = global->akm_sm;
		auth_pae = global->theAuthenticator;

		if( auth->Supp[i]->isEnable ){
//	        	printf("%s: CurrentAddress = %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
//					auth_pae->supp_addr[0], auth_pae->supp_addr[1], auth_pae->supp_addr[2],
//					auth_pae->supp_addr[3], auth_pae->supp_addr[4], auth_pae->supp_addr[5]	);

			if(akm_sm->IgnoreEAPOLStartCounter !=0 )
			{
				if(global->authSuccess){
					akm_sm->IgnoreEAPOLStartCounter = 0;
				}
				else{
					if( (akm_sm->IgnoreEAPOLStartCounter > 0) &&
						(akm_sm->IgnoreEAPOLStartCounter <= REJECT_EAPOLSTART_COUNTER) )
					{
						akm_sm->IgnoreEAPOLStartCounter--;
					}
				}

//				printf("%s: global->authSuccess = %d\n", __FUNCTION__, global->authSuccess);
//				printf("%s: IgnoreEAPOLStartCounter = %d\n", __FUNCTION__, akm_sm->IgnoreEAPOLStartCounter);
			}
			else{
			}
		}
		else{
//			printf("%s: auth->Supp[i]->isEnable = FALSE\n", __FUNCTION__);
		}
	}
}


int lib1x_akmsm_Update_Station_Status(Global_Params * global)
{

	int i;
	Dot1x_Authenticator * auth = global->auth;
	for(i=0 ;i < auth->MaxSupplicant ; i++)
	{
		if(global->auth->StaInfo[i].aid != 0)
		{
			if(!memcmp(auth->StaInfo[i].addr, global->theAuthenticator->supp_addr, MacAddrLen))
			{
				if(auth->StaInfo[i].tx_packets > global->theAuthenticator->acct_sm->tx_packets
				  || auth->StaInfo[i].rx_packets > global->theAuthenticator->acct_sm->rx_packets )
				{
					global->theAuthenticator->acct_sm->tx_packets = auth->StaInfo[i].tx_packets;
					global->theAuthenticator->acct_sm->rx_packets = auth->StaInfo[i].rx_packets;
					return akmsm_status_NotIdle;

				}else
				{
					return akmsm_status_Idle;
				}

			}

		}
	}
	return akmsm_status_NotInDriverTable;

}

void lib1x_akmsm_Account_Timer_proc(Dot1x_Authenticator * auth)
{
	int	i;
	int	iStationStatus;

	Global_Params *		global;
	APKeyManage_SM  *       akm_sm;

	//Get All Station Info if there is any station in session

	if(auth->IdleTimeoutEnabled)
	{

		lib1x_control_Query_All_Sta_Info(auth);

	}

	if(auth->AccountingEnabled)
		lib1x_acctsm(auth->authGlobal->global);


	//sc_yang
	for(i = 0 ; i < auth->MaxSupplicant ; i++)
        {
// reduce pre-alloc memory size, david+2006-02-06       
//		if(!auth->Supp[i]->isEnable)
		if(auth->Supp[i]==NULL || !auth->Supp[i]->isEnable)
			continue;

		global = auth->Supp[i]->global;
		akm_sm = global->akm_sm;

		//----------------------------------
		// Process accounting state machine
		//----------------------------------
		lib1x_acctsm( global);


		//----------------------------------
		// Process Session Timeout
		//----------------------------------
		if(auth->SessionTimeoutEnabled && global->akm_sm->SessionTimeoutEnabled)
		{

			//lib1x_PrintAddr(global->theAuthenticator->supp_addr);
			//lib1x_message(MESS_DBG_KEY_MANAGE,"STA[%d] SessionTimeoutCounter = %d, akm_sm->SessionTimeout = %d\n",
			//	global->index, akm_sm->SessionTimeoutCounter, auth->Supp[0]->global->akm_sm->SessionTimeout);

			// kenny
			//if(akm_sm->SessionTimeoutCounter <= akm_sm->SessionTimeout )
			if(akm_sm->SessionTimeoutCounter < akm_sm->SessionTimeout )
			{
				akm_sm->SessionTimeoutCounter++;
				//lib1x_message(MESS_DBG_KEY_MANAGE, "======================================================\n");
			}else
			{
				lib1x_message(MESS_DBG_KEY_MANAGE,"Kick of STATION because of session timeout\n");
				global->akm_sm->ErrorRsn = session_timeout;
				global->EventId = akmsm_EVENT_Disconnect;
				lib1x_akmsm_Disconnect( global );

			}
		}

		//----------------------------------
		// Process Interim Update
		//----------------------------------
		if(auth->AccountingEnabled && auth->UpdateInterimEnabled && global->akm_sm->InterimTimeoutEnabled)
		{
			//lib1x_message(MESS_DBG_KEY_MANAGE,"STA[%d] InterimTimeoutCounter = %d, akm_sm->InterimTimeout = %d\n",
			//		global->index, akm_sm->InterimTimeoutCounter, auth->Supp[0]->global->akm_sm->InterimTimeout);
			if(akm_sm->InterimTimeoutCounter >= akm_sm->InterimTimeout)
			{

				//global->theAuthenticator->acct_sm->action = acctsm_Interim_On;
				lib1x_acctsm_request(global, acctsm_Interim_On, 0);
				akm_sm->InterimTimeoutCounter = 0;
			}else
				akm_sm->InterimTimeoutCounter++;
		}



		global->theAuthenticator->acct_sm->elapsedSessionTime++;

		//----------------------------------
		// Process Idle Timeout
		//----------------------------------
		if(auth->IdleTimeoutEnabled && global->akm_sm->IdleTimeoutEnabled)
		{

			iStationStatus = lib1x_akmsm_Update_Station_Status(global);
			if(iStationStatus == akmsm_status_Idle || iStationStatus == akmsm_status_NotInDriverTable)
			{
				global->akm_sm->IdleTimeoutCounter--;
				//lib1x_message(MESS_DBG_KEY_MANAGE, "STA is Idle, global->akm_sm->IdleTimeoutCounter = %d\n", global->akm_sm->IdleTimeoutCounter);
				if(global->akm_sm->IdleTimeoutCounter == 0)
				{
					global->akm_sm->ErrorRsn = inactivity;//Idle Time out
					global->EventId = akmsm_EVENT_Disconnect;
					lib1x_message(MESS_DBG_KEY_MANAGE,"Kick of STATION because of Idle Timeout\n");
					lib1x_akmsm_Disconnect( global );
					lib1x_acctsm_request(global, acctsm_Acct_Stop, LIB1X_ACCT_REASON_IDLE_TIMEOUT);

				}


			}else if(iStationStatus == akmsm_status_NotIdle)
			{
				//lib1x_message(MESS_DBG_KEY_MANAGE, "STA is not Idle, global->akm_sm->IdleTimeoutCounter = %d\n", global->akm_sm->IdleTimeoutCounter);
				global->akm_sm->IdleTimeoutCounter = global->akm_sm->IdleTimeout;

			}
		}

	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
int lib1x_akmsm_AuthenticationRequest( Global_Params * global)
{
        APKeyManage_SM  *       akm_sm = global->akm_sm;
        int retVal = TRUE;
	static unsigned int RC_toggle = 0;

	//----------------------------------
	// Clear all state to initial value
	//----------------------------------
	lib1x_reset_authenticator(global);

	if( global->EventId == akmsm_EVENT_AuthenticationRequest)
	{
		// 802.11i/D3.0 p.95
		akm_sm->CurrentReplayCounter.field.HighPart = 0;
		akm_sm->CurrentReplayCounter.field.LowPart = 0;

		// For some STA that can only process if Replay Counter is not 0
		if((RC_toggle++)%2)
			INCLargeInteger(&akm_sm->CurrentReplayCounter);

		memset(akm_sm->PTK, 0, sizeof(akm_sm->PTK));
		lib1x_message(MESS_DBG_KEY_MANAGE, "*************************Process Event akmsm_EVENT_AuthenticationRequest\n");
		lib1x_message(MESS_DBG_KEY_MANAGE, "*************************CALL lib1x_control_RemovePTK\n");
		lib1x_control_RemovePTK(global, DOT11_KeyType_Pairwise);
		if(global->AuthKeyMethod == DOT11_AuthKeyType_RSN ||
			global->AuthKeyMethod == DOT11_AuthKeyType_RSNPSK ||
			global->AuthKeyMethod == DOT11_AuthKeyType_NonRSN802dot1x)
		{
			lib1x_control_SetPORT(global, DOT11_PortStatus_Unauthorized);
		}
	}

	if(global->EventId == akmsm_EVENT_AuthenticationRequest
		|| global->EventId == akmsm_EVENT_ReAuthenticationRequest )
	{
		lib1x_message(MESS_DBG_KEY_MANAGE, "*************************Process Event akmsm_EVENT_AuthenticationRequest and akmsm_EVENT_ReAuthenticationRequest\n");
		INCOctet32_INTEGER(&global->auth->Counter);
#ifndef RTL_WPA2_PREAUTH
		SetNonce(akm_sm->ANonce, global->auth->Counter);
#endif
	}

	if(global->AuthKeyMethod == DOT11_AuthKeyType_RSNPSK)
	{
		if( global->PreshareKeyAvaliable)//Always TRUE
		{
			memcpy(akm_sm->PMK, global->PSK, sizeof(global->PSK));
#ifdef RTL_WPA2
			CalcPMKID(	akm_sm->PMKID,
					akm_sm->PMK, 	 // PMK
					global->theAuthenticator->global->TxRx->oursupp_addr,   // AA
					global->theAuthenticator->supp_addr); 			// SPA
#endif
			akm_sm->state = akmsm_PTKSTART;

			//send 1st message
			lib1x_akmsm_SendEAPOL_proc(global);
		}
	}
	else if(global->AuthKeyMethod == DOT11_AuthKeyType_RSN)
	{
#ifdef RTL_WPA2
		if (!global->RSNVariable.isPreAuth
		    && global->RSNVariable.PMKCached) {
			memcpy(akm_sm->PMK, global->RSNVariable.cached_pmk_node->pmksa.pmk, PMK_LEN);
			wpa2_hexdump("Cached PMKID", global->RSNVariable.cached_pmk_node->pmksa.pmkid, PMKID_LEN);
			//wpa2_hexdump("Cached PMK", akm_sm->PMK, PMK_LEN);
			akm_sm->state = akmsm_PTKSTART;

			//send 1st message
			lib1x_akmsm_SendEAPOL_proc(global);

		} else {
#endif
		akm_sm->state = akmsm_AUTHENTICATION2;

#ifdef RTL_WPA2
		}
#endif
	}


	//If Receive Association Request, discard eapol-start message in 3 seconds
	global->akm_sm->IgnoreEAPOLStartCounter = REJECT_EAPOLSTART_COUNTER;

	return retVal;
}


int lib1x_akmsm_AuthenticationSuccess( Global_Params * global)
{
	APKeyManage_SM  *       akm_sm = global->akm_sm;
	int retVal = TRUE;


	lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_AuthenticationSuccess");

	if((global->AuthKeyMethod == DOT11_AuthKeyType_RSN || global->AuthKeyMethod == DOT11_AuthKeyType_NonRSN802dot1x) &&
		global->authSuccess && akm_sm->state == akmsm_AUTHENTICATION2)
	{
		//TODO*****ONLY FOR TEST*****
		global->RadiusKey.Status = MPPE_SDRCKEY_AVALIABLE;

		if( global->RadiusKey.Status == MPPE_SDRCKEY_AVALIABLE)
		{
#ifdef RTL_WPA2
                        struct _WPA2_PMKSA_Node* pmksa_node;
			pmksa_node = get_pmksa_node();
#endif
			lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_AuthenticationSuccess:Radius Key is Avaliable");
			memcpy(akm_sm->PMK, global->RadiusKey.RecvKey.Octet, global->RadiusKey.RecvKey.Length);
			//lib1x_control_AssociationRsp(global, DOT11_Association_Success);
#ifdef RTL_WPA2
			//printf("\n802.1x authentication done\n");
			CalcPMKID(	akm_sm->PMKID,
					akm_sm->PMK, 	 // PMK
					global->theAuthenticator->global->TxRx->oursupp_addr,   // AA
					global->theAuthenticator->supp_addr); 			// SPA
			//printf("Before cache_pmk\n");
			//dump_pmk_cache();
			// Save this PMKSA
			if (pmksa_node != NULL) {
				memcpy(pmksa_node->pmksa.pmkid, akm_sm->PMKID, PMKID_LEN);
				memcpy(pmksa_node->pmksa.pmk, akm_sm->PMK, PMK_LEN);
				memcpy(pmksa_node->pmksa.spa, global->theAuthenticator->supp_addr, ETHER_ADDRLEN);
				pmksa_node->pmksa.akmp = global->AuthKeyMethod;
				cache_pmksa(pmksa_node);
			} else {
				printf("%s:%d, pmksa_node == NULL\n", __FUNCTION__, __LINE__);
				exit(1);
			}
			//printf("After cache_pmk\n");
			//dump_pmk_cache();

			if ( global->RSNVariable.isPreAuth) {
				wpa2_hexdump("PreAuth done: ", global->theAuthenticator->supp_addr, ETHER_ADDRLEN);
				global->RSNVariable.isPreAuth = FALSE;
				lib1x_del_supp(global->auth, global->theAuthenticator->supp_addr);
			} else {
				akm_sm->state = akmsm_PTKSTART;
				//send 1st message
				//sleep(1);
				lib1x_akmsm_SendEAPOL_proc(global);
			}

#else
			akm_sm->state = akmsm_PTKSTART;
			//send 1st message
			//sleep(1);
			lib1x_akmsm_SendEAPOL_proc(global);
#endif
		}

	}

	return retVal;
}


int lib1x_akmsm_Disconnect( Global_Params * global)
{
	Auth_Pae                * auth_pae = global->theAuthenticator;
	APKeyManage_SM	*	akm_sm = global->akm_sm;
	int retVal = TRUE;

	lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_Disconnect(1)\n");


	//Disconnect is request from 1x daemon, Disassociate if indication from driver
	if(global->EventId == akmsm_EVENT_Disconnect || global->EventId == akmsm_EVENT_Disassociate)
	{
		if(global->EventId == akmsm_EVENT_Disconnect)
			lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_Disconnect(2) :\
				       	Request from 802.1x daemon\n");
		else if(global->EventId == akmsm_EVENT_Disassociate)
			lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_Disconnect(2) :\
                                        Request from wlan driver\n");


		if(global->auth->AccountingEnabled)
		{
			if(global->theAuthenticator->acct_sm->status == acctsm_Start)
			{
				if(global->EventId == akmsm_EVENT_Disconnect)
				{
					//Query tx/rx packet and byte counts
					lib1x_control_QuerySTA(auth_pae->global);
				}
				 else if(global->EventId == akmsm_EVENT_Disassociate)
				{
				 	//tx/rx packet bytes count has already been recorded
				}

				lib1x_acctsm_request(global, acctsm_Acct_Stop, lib1x_acct_maperr_wlan2acct(global->akm_sm->ErrorRsn));

			}
		}

		if(global->EventId == akmsm_EVENT_Disconnect)
		{

#ifdef	_ABOCOM
			lib1x_abocom(global->theAuthenticator->supp_addr, ABOCOM_DEL_STA);

#else
			lib1x_control_STADisconnect(global, global->akm_sm->ErrorRsn);
#endif
		}else if(global->EventId == akmsm_EVENT_Disassociate)
		{

#ifdef _ABOCOM
			//For Abocom Expiration mechanism
			//After Driver indicating an expiration with disassociate event,
			//MAC-Link is disconnected, Daemon-session is kept

			lib1x_abocom(global->theAuthenticator->supp_addr, ABOCOM_DEL_STA);
			lib1x_message(MESS_DBG_KEY_MANAGE, "Disassociate(expire) happend, keep session");
			return 1;
#endif
		}

	}
#if 0	//sc_yang move to later for delete supplicant
	if(global->EventId == akmsm_EVENT_Disconnect ||
	   global->EventId == akmsm_EVENT_Disassociate ||
		global->EventId ==  akmsm_EVENT_DeauthenticationRequest)
	{

//		PRINT_MAC_ADDRESS(global->theAuthenticator->supp_addr, "global->theAuthenticator->supp_addr");

		lib1x_del_supp(global->auth, global->theAuthenticator->supp_addr);

//2003-09-07
#if 0

		//---- Update variable in RTLAuthenticator ----
		global->auth->Supp[global->index]->isEnable = FALSE;
		lib1x_message(MESS_DBG_KEY_MANAGE, "Delete STA from Table");

		global->akm_sm->CurrentReplayCounter.field.HighPart = 0;
		global->akm_sm->CurrentReplayCounter.field.LowPart = 0;

		lib1x_message(MESS_DBG_KEY_MANAGE, "Number of Supplicant = %d\n",global->auth->NumOfSupplicant);
//		global->auth->NumOfSupplicant --;

		//0818
		//lib1x_get_NumSTA(global->auth);
#endif
	}
#endif



	memset(akm_sm->PMK, 0, sizeof(akm_sm->PMK));

	global->auth->gk_sm->GInitAKeys = FALSE;
	akm_sm->PInitAKeys = FALSE;
	akm_sm->IntegrityFailed = FALSE;

	if(global->auth->RSNVariable.isSupportUnicastCipher && global->RSNVariable.isSuppSupportUnicastCipher)
		akm_sm->Pair = TRUE;
		if(global->EventId != akmsm_EVENT_Disassociate) //sc_yang
			lib1x_control_RemovePTK(global, DOT11_KeyType_Pairwise);


	//---- Initialize 802.1x related variable ----

	if(global->AuthKeyMethod == DOT11_AuthKeyType_RSN ||
		global->AuthKeyMethod == DOT11_AuthKeyType_RSNPSK ||
		global->AuthKeyMethod == DOT11_AuthKeyType_NonRSN802dot1x)
		if(global->EventId != akmsm_EVENT_Disassociate)	//sc_yang
			lib1x_control_SetPORT(global, DOT11_PortStatus_Unauthorized);


	//sc_yang :delete supplicant last
	if(global->EventId == akmsm_EVENT_Disconnect ||
	   global->EventId == akmsm_EVENT_Disassociate ||
		global->EventId ==  akmsm_EVENT_DeauthenticationRequest)
	{

//		PRINT_MAC_ADDRESS(global->theAuthenticator->supp_addr, "global->theAuthenticator->supp_addr");

#if !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL865X_SC) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196C_EC)
		lib1x_del_supp(global->auth, global->theAuthenticator->supp_addr);
#endif

//2003-09-07
#if 0

		//---- Update variable in RTLAuthenticator ----
		global->auth->Supp[global->index]->isEnable = FALSE;
		lib1x_message(MESS_DBG_KEY_MANAGE, "Delete STA from Table");

		global->akm_sm->CurrentReplayCounter.field.HighPart = 0;
		global->akm_sm->CurrentReplayCounter.field.LowPart = 0;

		lib1x_message(MESS_DBG_KEY_MANAGE, "Number of Supplicant = %d\n",global->auth->NumOfSupplicant);
//		global->auth->NumOfSupplicant --;

		//0818
		//lib1x_get_NumSTA(global->auth);
#endif
	}
	akm_sm->state = akmsm_INITIALIZE;
	global->EventId = akmsm_EVENT_NoEvent;
	global->akm_sm->SessionTimeoutCounter = 0;
	global->akm_sm->IdleTimeoutCounter = 0;
	global->akm_sm->InterimTimeoutCounter = LIB1X_DEFAULT_IDLE_TIMEOUT;




	return retVal;
}


int lib1x_akmsm_EAPOLKeyRecvd( Global_Params * global)
{
	APKeyManage_SM	*	akm_sm = global->akm_sm;
	int retVal = 0, result = -1;

	global->EapolKeyMsgRecvd.Octet = global->EAPOLMsgRecvd.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN;


	//----IEEE 802.11-03/156r2. MIC report : (1)MIC bit (2)error bit (3) request bit
	//----Check if it is MIC failure report. If it is, indicate to driver
	if(Message_KeyMIC(global->EapolKeyMsgRecvd) && Message_Error(global->EapolKeyMsgRecvd)
		&& Message_Request(global->EapolKeyMsgRecvd))
	{
		lib1x_control_IndicateMICFail(global->auth, global->theAuthenticator->supp_addr);
		return retVal;
	}


	if(Message_KeyType(global->EapolKeyMsgRecvd) == type_Pairwise)
	{

		switch(akm_sm->state)
		{
		case akmsm_PTKSTART:

			//---- Receive 2nd message and send third
			if(!(result = lib1x_akmsm_ProcessEAPOL_proc(global)))
			{
				lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_EAPOLKeyRecvd:Receive EAPOL-KEY and check [successfully] in akmsm_PTKSTART state");
				akm_sm->state = akmsm_PTKINITNEGOTIATING;
				global->akm_sm->bWaitForPacket = FALSE;
				lib1x_akmsm_SendEAPOL_proc(global);
			}else
			{
				lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_EAPOLKeyRecvd:Receive EAPOL-KEY and check [Fail] in akmsm_PTKSTART state");
			}

			break;

		case akmsm_PTKINITNEGOTIATING:

			//---- Receive 4th message ----
			if(!(result = lib1x_akmsm_ProcessEAPOL_proc(global)))
			{
				lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_EAPOLKeyRecvd:Receive EAPOL-KEY and check [successfully] in akmsm_PTKINITNEGOTIATING state");
				//if( akm_sm->Pair)
				lib1x_control_SetPTK(global);
				lib1x_control_SetPORT(global, DOT11_PortStatus_Authorized);
				global->auth->gk_sm->GInitAKeys = TRUE;
				akm_sm->PInitAKeys = TRUE;
				akm_sm->state = akmsm_PTKINITDONE;
				global->akm_sm->bWaitForPacket = FALSE;

				//lib1x_akmsm_UpdateGK_proc() calls lib1x_akmsm_SendEAPOL_proc for 2-way
				//if group key sent is needed, send msg 1 of 2-way handshake
#ifdef RTL_WPA2
				if ( global->RSNVariable.WPA2Enabled ) {
					lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_EAPOLKeyRecvd : Receive 4 message of 4-way handshake and check successfully\n");
					global->akm_sm->bWaitForPacket = FALSE;
					//------------------------------------------------------
					// Only when the group state machine is in the state of
					// (1) The first STA Connected,
					// (2) UPDATE GK to all station
					// does the GKeyDoneStations needed to be decreased
					//------------------------------------------------------

					if(global->auth->gk_sm->GKeyDoneStations > 0)
						global->auth->gk_sm->GKeyDoneStations--;

					lib1x_message(MESS_DBG_KEY_MANAGE, " global->auth->gk_sm->GKeyDoneStations=%d\n",  global->auth->gk_sm->GKeyDoneStations);
					//Avaya akm_sm->TimeoutCtr = 0;
					//To Do : set port secure to driver
					global->portSecure = TRUE;
					//akm_sm->state = akmsm_PTKINITDONE;
					akm_sm->gstate = gkmsm_REKEYESTABLISHED;


					if( global->auth->gk_sm->GKeyDoneStations == 0 && !global->auth->gk_sm->GkeyReady)
			                {
		                	        if(lib1x_control_SetGTK(global) == 0)//success
						{
		                        		global->auth->gk_sm->GkeyReady = TRUE;
							global->auth->gk_sm->GResetCounter = TRUE;
						}
			                }

					if (global->RSNVariable.PMKCached) {
						global->RSNVariable.PMKCached = FALSE;  // reset
					}

					printf("WPA2: 4-way handshake done\n");
					//printf("-----------------------------------------------------------------------------\n\n\n\n\n\n\n");

				} else {
#ifdef FOURWAY_DEBUG
					//printf("\nkenny: global->RSNVariable.WPA2Enabled == FALSE\n");
#endif
#endif /* RTL_WPA2 */
				if(!Message_Secure(global->EapolKeyMsgRecvd))
					lib1x_akmsm_UpdateGK_proc(global->auth);
#ifdef RTL_WPA2
				}
#endif /* RTL_WPA2 */
			}else
			{
				if(result == ERROR_RECV_4WAY_MESSAGE2_AGAIN )
					lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_EAPOLKeyRecvd:Receive EAPOL-KEY 4way  message 2 again ");
				else
				lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_EAPOLKeyRecvd:Receive EAPOL-KEY and check [Fail] in akmsm_PTKINITNEGOTIATING state");
			}
			break;

		case akmsm_PTKINITDONE:

			if(!(result = lib1x_akmsm_ProcessEAPOL_proc(global)))
			{
				lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_EAPOLKeyRecvd:Receive EAPOL-KEY and check [successfully] in akmsm_PTKINITDONE state");
				global->akm_sm->bWaitForPacket = FALSE;
				//------------------------------------------------
				// Supplicant request to init 4 or 2 way handshake
				//------------------------------------------------
				if(Message_Request(global->EapolKeyMsgRecvd))
				{
					akm_sm->state = akmsm_PTKSTART;
					 if(Message_KeyType(global->EapolKeyMsgRecvd) == type_Pairwise)
					 {
						if(Message_Error(global->EapolKeyMsgRecvd))
							lib1x_akmsm_IntegrityFailure(global);
					 }else if(Message_KeyType(global->EapolKeyMsgRecvd) == type_Group)
					 {
						if(Message_Error(global->EapolKeyMsgRecvd))
						{
							//auth change group key, initilate 4-way handshake with supp and execute
							//the Group key handshake to all Supplicants
							global->auth->gk_sm->GKeyFailure = TRUE;
							lib1x_akmsm_IntegrityFailure(global);
						}
					 }

					 //---- Start 4-way handshake ----
					 lib1x_akmsm_SendEAPOL_proc(global);
				}
#ifdef RTL_WPA2_PREAUTH
				lib1x_message(MESS_DBG_KEY_MANAGE, "%s() in akmsm_PTKINITDONE state. Call lib1x_akmsm_UpdateGK_proc()\n", __FUNCTION__);
#endif
				//---- Execute Group Key state machine for each STA ----
				lib1x_akmsm_UpdateGK_proc(global->auth);
			}else
			{
				lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_EAPOLKeyRecvd:Receive EAPOL-KEY and check [Fail] in akmsm_PTKINITDONE state");
				lib1x_message(MESS_ERROR_OK, KM_STRERR(result));
			}

			break;
		default:
			break;

		}//switch

	}else if(Message_KeyType(global->EapolKeyMsgRecvd) == type_Group)
	{
		lib1x_message(MESS_DBG_KEY_MANAGE, "global->auth->gk_sm->GKeyDoneStations=%d\n", global->auth->gk_sm->GKeyDoneStations);

		//---- Receive 2nd message of 2-way handshake ----
		if(!(result = lib1x_akmsm_ProcessEAPOL_proc(global)))
		//Avaya
		//if(1)
		{

			lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_EAPOLKeyRecvd : Receive 2 message of 2-way handshake and check successfully\n");
			global->akm_sm->bWaitForPacket = FALSE;
			//------------------------------------------------------
			// Only when the group state machine is in the state of
			// (1) The first STA Connected,
			// (2) UPDATE GK to all station
			// does the GKeyDoneStations needed to be decreased
			//------------------------------------------------------

			if(global->auth->gk_sm->GKeyDoneStations > 0)
				global->auth->gk_sm->GKeyDoneStations--;

			lib1x_message(MESS_DBG_KEY_MANAGE, " global->auth->gk_sm->GKeyDoneStations=%d\n",  global->auth->gk_sm->GKeyDoneStations);
			//Avaya akm_sm->TimeoutCtr = 0;
			//To Do : set port secure to driver
			global->portSecure = TRUE;
			akm_sm->state = akmsm_PTKINITDONE;
			akm_sm->gstate = gkmsm_REKEYESTABLISHED;


			if( global->auth->gk_sm->GKeyDoneStations == 0 && !global->auth->gk_sm->GkeyReady)
	                {
                	        if(lib1x_control_SetGTK(global) == 0)//success
				{

					printf("2-way Handshake is finished\n");
                        		global->auth->gk_sm->GkeyReady = TRUE;
					global->auth->gk_sm->GResetCounter = TRUE;
				}
	                }

		}else
		{
			lib1x_message(MESS_DBG_KEY_MANAGE, " Receive bad group key handshake");
             		lib1x_message(MESS_ERROR_OK, KM_STRERR(result));
                }
	}

	// Deal with the erro when processing EAPOL-Key that may result in Disconnect
	// (1) RSN_MIC_failure (2)RSN_diff_info_element
	if(result)
	{
                lib1x_message(MESS_ERROR_OK, KM_STRERR(result));
		//To Do : do we need to send disassociate right now?

	}
	return retVal;
}

int lib1x_akmsm_IntegrityFailure( Global_Params * global)
{

	APKeyManage_SM	*	akm_sm = global->akm_sm;
	int retVal = TRUE;

	akm_sm->IntegrityFailed = FALSE;
	if(global->auth->gk_sm->GKeyFailure)
	{
		global->auth->gk_sm->GTKRekey = TRUE;
		global->auth->gk_sm->GKeyFailure = FALSE;
	}
	//waitupto60;

	INCOctet32_INTEGER(&global->auth->Counter);
	SetNonce(global->akm_sm->ANonce, global->auth->Counter);

	INCOctet32_INTEGER(&global->auth->Counter);
	SetNonce(global->akm_sm->ANonce, global->auth->Counter);

	return retVal;

}

//---------------------------------------------------------------------------------
// lib1x_akmsm_trans : Check if key manage state machine execution is required
//	according to whether there is external event or change of internal variable
// Retrun : TRUE is required/ FALSE is non-required
//---------------------------------------------------------------------------------
int lib1x_akmsm_trans(Global_Params * global)
{
        int     retVal = 0;


	if(global->AuthKeyMethod != DOT11_AuthKeyType_RSN &&
	   global->AuthKeyMethod != DOT11_AuthKeyType_RSNPSK )
		return retVal;

        switch(global->akm_sm->state)
        {
        case akmsm_AUTHENTICATION2:
		// Check global->theAuthenticator->rxRespId because when eapStart is set,
		// the pae state machine enters into CONNECTING state which do not clear
		// authSuccess to FALSE
		switch(global->AuthKeyMethod)
		{
		case DOT11_AuthKeyType_RSN:
			if(global->authSuccess && global->theAuthenticator->rxRespId ){
				global->EventId = akmsm_EVENT_AuthenticationSuccess;
				retVal = TRUE;
			}
			break;
		case DOT11_AuthKeyType_RSNPSK:
			global->EventId = akmsm_EVENT_AuthenticationSuccess;
			retVal = TRUE;
			break;
		case DOT11_AuthKeyType_NonRSN802dot1x:
			break;
		default:
			printf("%s: Unknown AuthKeyMethod\n", __FUNCTION__);
			break;
		}

                break;
	case akmsm_PTKSTART:
	case akmsm_PTKINITNEGOTIATING:
		switch(global->AuthKeyMethod)
		{
		case DOT11_AuthKeyType_RSN:
		case DOT11_AuthKeyType_RSNPSK:
			if(global->akm_sm->TimeoutEvt)
			{

				global->EventId = akmsm_EVENT_TimeOut;
				retVal = TRUE;
			}
			break;
		case DOT11_AuthKeyType_NonRSN802dot1x:
			break;
		}
	default:
		break;
        /*
        case akmsm_PTKSTART:
        case akmsm_PTKINITNEGOTIATING:
        case akmsm_PTKINITDONE:
                global->EventId = akmsm_EVENT_EAPOLKeyRecvd;
                retVal = TRUE;
        */
        }

	switch(global->akm_sm->gstate)
	{
	case gkmsm_REKEYNEGOTIATING:
		switch(global->AuthKeyMethod)
		{
		case DOT11_AuthKeyType_RSN:
		case DOT11_AuthKeyType_RSNPSK:
			if(global->akm_sm->TimeoutEvt)
			{
				global->EventId = akmsm_EVENT_TimeOut;
				retVal = TRUE;
			}
			break;
		case DOT11_AuthKeyType_NonRSN802dot1x:
			break;
		}//switch(global->AuthKeyMethod)
		break;
	}


//	PRINT_GLOBAL_AKM_SM_STATE(global);
//	PRINT_GLOBAL_AKM_SM_GSTATE(global);

	//ToDo : Check out the order of this event




	if( global->akm_sm->eapStart == TRUE)
	{

		if(global->akm_sm->state == akmsm_AUTHENTICATION2)
		//The first time to do authentication
		{
			lib1x_message(MESS_DBG_KEY_MANAGE, "Judge eapStart in state akmsm_AUTHENTICATION2\n");
			global->EventId = akmsm_EVENT_AuthenticationRequest;
		}
		else
		//Reauthentication because of supplicant sending eapol satrt
		{
			lib1x_message(MESS_DBG_KEY_MANAGE, "Judge eapStart not in state akmsm_AUTHENTICATION2\n");
			global->EventId = akmsm_EVENT_ReAuthenticationRequest;
		}
		global->akm_sm->eapStart = FALSE;
		retVal = TRUE;

	}

	//Besides checking eapol start message sending from client, we should also check
	//if Supplicnat send 802.11 authentication management
	//To prevent from client not sending eapol start.
	if( global->akm_sm->AuthenticationRequest == TRUE)
        {
		lib1x_message(MESS_DBG_KEY_MANAGE,"global->akm_sm->AuthenticationRequest == TRUE");
                global->EventId = akmsm_EVENT_AuthenticationRequest;
                global->akm_sm->AuthenticationRequest = FALSE;
                retVal = TRUE;
        }
        else if(global->akm_sm->DeauthenticationRequest == TRUE)
        {
                global->EventId = akmsm_EVENT_DeauthenticationRequest;
                global->akm_sm->DeauthenticationRequest = FALSE;
                retVal = TRUE;
        }
        else if(global->akm_sm->Disconnect == TRUE )
        {
#ifdef RTL_WPA2_PREAUTH
		//printf("%s-%d: global->EventId = akmsm_EVENT_Disconnect\n", __FUNCTION__,__LINE__);
#endif
                global->EventId = akmsm_EVENT_Disconnect;
                global->akm_sm->Disconnect = FALSE;
                retVal = TRUE;
        }

	if(retVal)
		lib1x_akmsm_dump(global);

//	PRINT_GLOBAL_EVENTID(global);

        return retVal;

}



void lib1x_akmsm_dump(Global_Params * global)
{
#ifdef ALLOW_DBG_KEY_MANAGE

	lib1x_message(MESS_DBG_KEY_MANAGE, "-------Dump Supplicant [%d] Key Manage State Machine-----------", global->index);
	lib1x_PrintAddr(global->theAuthenticator->supp_addr);
        switch(global->akm_sm->state)
        {
        case akmsm_AUTHENTICATION2 :    lib1x_message(MESS_DBG_KEY_MANAGE, "state :AUTHENTICATION2"); break;
        case akmsm_PTKSTART:            lib1x_message(MESS_DBG_KEY_MANAGE, "state :PTKSTAR"); break;
        case akmsm_PTKINITNEGOTIATING:  lib1x_message(MESS_DBG_KEY_MANAGE, "state :PTKINITNEGOTIATING"); break;
        case akmsm_PTKINITDONE:         lib1x_message(MESS_DBG_KEY_MANAGE, "state :PTKINITDONE"); break;
        }

	switch(global->akm_sm->gstate)
	{
	case gkmsm_REKEYNEGOTIATING:	lib1x_message(MESS_DBG_KEY_MANAGE, "gstate :REKEYNEGOTIATING"); break;
	case gkmsm_REKEYESTABLISHED:	lib1x_message(MESS_DBG_KEY_MANAGE, "gstate :REKEYESTABLISHED"); break;
	case gkmsm_KEYERROR:		lib1x_message(MESS_DBG_KEY_MANAGE, "gstate :KEYERROR"); break;

	}

	switch( global->EventId )
	{

        case    akmsm_EVENT_NoEvent:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :NoEvent");
		break;
        case    akmsm_EVENT_AssociationRequest:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :AssociationRequest");
		break;
        case    akmsm_EVENT_ReAssociationRequest:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :ReAssociationRequest");
		break;
        case    akmsm_EVENT_AuthenticationRequest:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :AuthenticationRequest");
		break;
        case    akmsm_EVENT_ReAuthenticationRequest:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :ReAuthenticationRequest");
		break;
        case    akmsm_EVENT_AuthenticationSuccess:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :AuthenticationSuccess");
		break;
        case    akmsm_EVENT_Disconnect:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :Disconnect");
		break;
        case    akmsm_EVENT_DeauthenticationRequest:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :DeauthenticationRequest");
		break;
        case    akmsm_EVENT_Init:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :Init");
		break;
        case    akmsm_EVENT_IntegrityFailure:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :IntegrityFailure");
		break;
        case    akmsm_EVENT_EAPOLKeyRecvd:
		lib1x_message(MESS_DBG_KEY_MANAGE, "event :EAPOLKeyRecvd");
		break;
        }//switch


	switch(global->AuthKeyMethod)
	{
	case 	DOT11_AuthKeyType_RSN:
		lib1x_message(MESS_DBG_KEY_MANAGE, "AuthKeyMethod :RSN");
		break;
        case 	DOT11_AuthKeyType_RSNPSK:
		lib1x_message(MESS_DBG_KEY_MANAGE, "AuthKeyMethod :RSNPSK");
		break;
        case 	DOT11_AuthKeyType_NonRSN802dot1x:
		lib1x_message(MESS_DBG_KEY_MANAGE, "AuthKeyMethod :NonRSN802dot1x");
		break;
	case    DOT11_AuthKeyType_PRERSN:
		lib1x_message(MESS_DBG_KEY_MANAGE, "AuthKeyMethod :PRERSN");
		break;

	}
	lib1x_message(MESS_DBG_KEY_MANAGE, "AuthenticationRequest : %d", global->akm_sm->AuthenticationRequest);
	lib1x_message(MESS_DBG_KEY_MANAGE, "ReAuthenticationRequest : %d", global->akm_sm->ReAuthenticationRequest);
	lib1x_message(MESS_DBG_KEY_MANAGE, "DeauthenticationRequest : %d", global->akm_sm->DeauthenticationRequest);
	lib1x_message(MESS_DBG_KEY_MANAGE, "Disconnect : %d", global->akm_sm->Disconnect);
	lib1x_message(MESS_DBG_KEY_MANAGE, "Init : %d", global->akm_sm->Init);
	//lib1x_message(MESS_DBG_KEY_MANAGE, " : %d", global->akm_sm->);
	//lib1x_message(MESS_DBG_KEY_MANAGE, " : %d", global->akm_sm->);
	//lib1x_message(MESS_DBG_KEY_MANAGE, " : %d", global->akm_sm->);
	//lib1x_message(MESS_DBG_KEY_MANAGE, " : %d", global->akm_sm->);
	//lib1x_message(MESS_DBG_KEY_MANAGE, " : %d", global->akm_sm->);




	lib1x_message(MESS_DBG_KEY_MANAGE, "------------------------------------------------------------\n",
global->index);
#endif
}


void lib1x_akmsm_execute( Global_Params * global)
{

        Auth_Pae                * auth_pae;
        struct lib1x_eapol 	* eapol;
        struct lib1x_ethernet 	* eth_hdr;

	BOOLEAN	bFlag = FALSE;

	auth_pae = global->theAuthenticator;

//	PRINT_GLOBAL_EVENTID(global);


	switch( global->EventId )
	{


	case	akmsm_EVENT_AuthenticationRequest:
	case	akmsm_EVENT_ReAuthenticationRequest:
		lib1x_akmsm_AuthenticationRequest( global );
		break;

        case    akmsm_EVENT_AuthenticationSuccess:
                lib1x_akmsm_AuthenticationSuccess(global);
                break;

	case	akmsm_EVENT_Disconnect:
	case	akmsm_EVENT_DeauthenticationRequest:
	case    akmsm_EVENT_Init:
	case    akmsm_EVENT_Disassociate:
		lib1x_akmsm_Disconnect( global );
		break;

	case	akmsm_EVENT_IntegrityFailure:
		lib1x_akmsm_IntegrityFailure( global );
		break;
	case	akmsm_EVENT_EAPOLKeyRecvd:
		bFlag = TRUE;
		lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_execute: Receive EAPOL Key\n");
		lib1x_akmsm_EAPOLKeyRecvd( global );
		break;

	case    akmsm_EVENT_TimeOut:
		auth_pae->sendhandshakeready = TRUE;
		//ToDo : Update Replay Counter ( or not )
		break;
	}//switch

	global->EventId = akmsm_EVENT_NoEvent;


	if( auth_pae->sendhandshakeready )
        {
		lib1x_message(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_execute: Has EAPOL Key Packet sent to supplicant\n");
		lib1x_PrintAddr(global->theAuthenticator->supp_addr);
                //ethernet and eapol header initialization
                eth_hdr = ( struct lib1x_ethernet * )global->EAPOLMsgSend.Octet;
                memcpy ( eth_hdr->ether_dhost , auth_pae->supp_addr, ETHER_ADDRLEN );
	        memcpy ( eth_hdr->ether_shost , auth_pae->global->TxRx->oursupp_addr, ETHER_ADDRLEN );
		eth_hdr->ether_type = htons(LIB1X_ETHER_EAPOL_TYPE);

                eapol = ( struct lib1x_eapol * )  ( global->EAPOLMsgSend.Octet +  ETHER_HDRLEN )  ;
                eapol->protocol_version = LIB1X_EAPOL_VER;
                eapol->packet_type =  LIB1X_EAPOL_KEY;

                eapol->packet_body_length = htons(global->EapolKeyMsgSend.Length);

		//lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_execute", auth_pae->sendBuffer, global->EAPOLMsgSend.Length, "Send EAPOL-KEY Message");

		if(global->akm_sm->IfCalcMIC)
		{
			CalcMIC(global->EAPOLMsgSend, global->KeyDescriptorVer, global->akm_sm->PTK, PTK_LEN_EAPOLMIC);
			global->akm_sm->IfCalcMIC = FALSE;
		}
		global->akm_sm->TimeoutEvt = 0;
                //global->akm_sm->TimeoutCtr = 0;


		//lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_execute", auth_pae->sendBuffer, global->EAPOLMsgSend.Length, "AP Send EAPOL-KEY Message");

		lib1x_nal_send( auth_pae->global->TxRx->network_supp, auth_pae->sendBuffer, global->EAPOLMsgSend.Length );
                auth_pae->sendhandshakeready = FALSE;
		global->akm_sm->bWaitForPacket = TRUE;
        }
}
