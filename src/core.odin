package perihelion

import "core:fmt"
import "core:os"

main :: proc() {
	if (len(os.args) != 2) {
		fmt.printf("usage: ./comet pathtoexecutable\n")
		return
	}

	file, readstatus := os.read_entire_file(os.args[1])
	if (!readstatus) {
		fmt.printf("failed to open file: {}\n", os.args[1])
		return
	}
	fmt.printf("file contents: \n{}\n", string(file));
	decode(string(file))
}

decode :: proc(s: string) {
	//determine which type of instruction we're dealing with

}

instruction_format :: enum {
	TYPE_R,
	TYPE_M,
	TYPE_F,
	TYPE_J,
	TYPE_B
}

instructions := map[int][instruction_type] {

}