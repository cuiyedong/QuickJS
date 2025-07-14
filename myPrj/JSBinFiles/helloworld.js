/*
 * ======================================================
 *
 *  此文件用来测试.js文件的执行，包括如下功能：
 *      1. 自定义的函数
 *
 * ======================================================
 */
console.log("====  helloworld.js invoke test  ====");
console.log("mmath fun test, 4 + 5 = ", mmath.add(4,5))
console.log("mmath var test, pai = ", mmath.pai)

console.log("selfdef object test:")
let obj  = new Point(3,4)
console.log(obj.norm())
obj.x = 6
obj.y = 8
console.log(obj.norm())

console.log("\r\n")
