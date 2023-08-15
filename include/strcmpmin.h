#ifndef __STRCMPMIN_H
#define __STRCMPMIN_H

byte strcmp(const char *cs, const char *ct) {
	while ((*cs != 0) && (*ct != 0)) {
		if (*cs++ != *ct++) return 1;
	}
	if (*cs != *ct) return 1;
	return 0;
}

byte strncmp(const char *cs, const char *ct, byte n) {
	while ((*cs != 0) && (*ct != 0) && (n > 0)) {
		if (*cs++ != *ct++) return 1;
		n--;
	}
	if (n == 0) return 0;
	
	if (*cs != *ct) return 1;
	return 0;
}

#endif	// __STRCMPMIN_H
