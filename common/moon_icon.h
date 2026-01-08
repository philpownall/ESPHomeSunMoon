// Thanks to https://community.home-assistant.io/u/Tofandel
const char * moon_icon(int moon_age) {
	if (moon_age < 2)
       return "󰽧";  // moon-new
	else if (moon_age < 7)
       return "󰽧"; // moon-waxing-crescent
	else if (moon_age < 8)
       return "󰽡"; // moon-first-quarter
	else if (moon_age < 13)
       return "󰽨"; // moon-waxing-gibbous
	else if (moon_age < 16)
       return "󰽢"; // moon-full
	else if (moon_age < 22)
       return "󰽦"; // moon-waning-gibbous
	else if (moon_age < 23)
       return "󰽣"; // moon-last-quarter
	else if (moon_age < 28)
       return "󰽥"; // moon-waning-crescent
	else
       return "󰽧"; // moon-new
}
