import subprocess
import time

# 用于调用exe进行测试

# 设置可执行文件的路径
exe_path = r"D:\Code\CppProj\cppTestProj\build\OpenGL\Bezier\Debug\OpenGL_Bezier_curve_outpt.exe"

# 启动20次
for i in range(20):
    print(f"启动第 {i + 1} 次...")
    subprocess.Popen(exe_path)  # 使用Popen启动进程，不会阻塞当前脚本
    time.sleep(0.5)  # 稍作延迟，避免启动过