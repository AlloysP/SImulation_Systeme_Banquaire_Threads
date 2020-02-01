/**
 * Authors: PETIT Alloys, JOSSE Raphaël
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char *msgToPrintConvert(char *pMsg)
{
    char vMsg[50];
    char *msgToPrint = malloc(50 * sizeof(char));
    int vMsgSize;

    sprintf(vMsg, "%s", pMsg);
    vMsgSize = strlen(vMsg);
    vMsg[vMsgSize - 1] = '\0';
    vMsg[vMsgSize] = 0;
    sprintf(msgToPrint, "%s", vMsg);
    return msgToPrint;
}