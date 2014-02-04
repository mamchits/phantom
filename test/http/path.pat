"/" -> "/"
"/./" -> "/"
"/../" -> "/"
"/." -> "/"
"/.." -> "/"
"/abaaa/ccca/dddd/../qqqqq/../pqqr" -> "/abaaa/ccca/pqqr"
"/abaaa/ccca/dddd/../qqqqq/../../pqqr" -> "/abaaa/pqqr"
"/abaaa/ccca/dddd/./qqqqq/././pqqr" -> "/abaaa/ccca/dddd/qqqqq/pqqr"
"/abaaa/ccca/dddd//qqqqq///pqqr" -> "/abaaa/ccca/dddd/qqqqq/pqqr"
"/abaaa/ccca/dddd//qqqqq///pqqr/.././../.." -> "/abaaa/ccca/"
"/abaaa/ccca/dddd/../qqqqq%2F%2E%2E%2F%2E%2E%2Fpqqr" -> "/abaaa/pqqr"
"/%2F%2E%2E%2F%2E%2E%2Fpqqr" -> "/pqqr"
400 Wrong symbol in path
"/%pqqr" -> Error
400 Wrong symbol in path
"/%" -> Error
"/.ewrqewr/qweeee/" -> "/.ewrqewr/qweeee/"
"/wwww/..qweqw/qqq" -> "/wwww/..qweqw/qqq"
