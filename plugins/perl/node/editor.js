const editor = require('editor');

const { EEC_MODIFIED } = require('editor');


async function main() {
    const res = await editor.askUser("Enter something:");
    if (res.ok) {
        console.log("User typed:", res.text);
    }
}

main();

editor.advControl(0, CMD_SOMETHING, 0, 0);
