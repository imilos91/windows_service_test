#pragma once

#include <iostream>
#include <stdio.h>  
#include <assert.h>  
#include <windows.h>   
#include <lm.h>  

using namespace std;

VOID DisplayLocalLogons(LPTSTR ServerName);

VOID DisplaySessionLogons(LPTSTR ServerName);