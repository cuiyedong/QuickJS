/*
 * ======================================================
 *
 *  此文件用来测试cCallback.c文件的执行，包括如下功能：
 *
 * ======================================================
 */
console.log("====  cCallback invoke test  ====");
let button = gui.createWidget(gui.BUTTON, {
    x : 100,
    onClick : (x, y) => {
        console.log(`js onclick ${x} ${y}`)
    }
})

button.mockClick(15, 30)
gui.deleteWidget(button)

console.log("\r\n")
