import os
from shutil import copyfile

source_dir = 'D:/CAD/'           # 仓库源路径
paster_dir = '.' # 新路径 / 还原路径


class HandleFile:
    def __init__(self, copy_dir, paster_dir, ignore_dir):
        self.copy_dir   = copy_dir
        self.paster_dir = paster_dir
        self.ignore_dir = set(ignore_dir)   # 用集合加速查找
        self.add_suffix = '_aprilxx'

    # 判断是否为文本文件
    def __is_txt_file(self, fileName: str) -> bool:
        try:
            with open(fileName, 'r', encoding='utf-8') as _:
                return True
        except (UnicodeDecodeError, OSError):
            return False

    # ------------------------------------------------------------------
    # 复制文件（已修复：1. 真正忽略目录  2. 正确拼接路径）
    # ------------------------------------------------------------------
    def copy_files(self):
        print(f'\n开始复制文件，源路径：{self.copy_dir}')
        print(f'目标路径：{self.paster_dir}')
        count = 0
        err   = []
        prefix_len = len(self.copy_dir)

        for root, dirs, files in os.walk(self.copy_dir, topdown=True):
            root = root.replace('\\', '/')

            # 1) 真正忽略目录：把子目录从遍历列表中剔除
            dirs[:] = [d for d in dirs if d not in self.ignore_dir]
            if os.path.basename(root) in self.ignore_dir:
                continue

            # 2) 正确拼接目标目录
            dest_dir = os.path.join(self.paster_dir, root[prefix_len:].lstrip('/'))
            os.makedirs(dest_dir, exist_ok=True)

            for name in files:
                count += 1
                src = os.path.join(root, name)
                ext = os.path.splitext(name)[1].lower()

                try:
                    if self.__is_txt_file(src) or ext == '.idx':
                        dst = os.path.join(dest_dir, name + self.add_suffix)
                        with open(src, 'rb') as f1, open(dst, 'wb') as f2:
                            f2.write(f1.read())
                        print(f'复制并加后缀：{src} -> {dst}')
                    else:
                        dst = os.path.join(dest_dir, name)
                        copyfile(src, dst)
                        print(f'直接复制：{src} -> {dst}')
                except Exception as e:
                    print(f'复制失败：{src}，{e}')
                    err.append(src)

        print(f'\n复制完成，共处理 {count} 个文件')
        if err:
            print('\n失败的文件：', *err, sep='\n')

    # ------------------------------------------------------------------
    # 还原文件（去掉后缀）
    # ------------------------------------------------------------------
    def rnameSuffix(self):
        print(f'\n开始还原文件，路径：{self.paster_dir}')
        remove_len = len(self.add_suffix)
        count = 0
        err   = []

        for root, _, files in os.walk(self.paster_dir):
            for name in files:
                if name.endswith(self.add_suffix):
                    old = os.path.join(root, name)
                    new = old[:-remove_len]
                    try:
                        if os.path.exists(new):
                            os.remove(old)
                            print(f'目标已存在，删除：{old}')
                        else:
                            os.rename(old, new)
                            print(f'还原：{old} -> {new}')
                    except Exception as e:
                        print(f'还原失败：{old}，{e}')
                        err.append(old)
                    count += 1

        print(f'\n还原完成，共处理 {count} 个文件')
        if err:
            print('\n失败的文件：', *err, sep='\n')


# ======================================================================
# 交互主程序（保持不变）
# ======================================================================
if __name__ == '__main__':
    print('-------------------------\n----------Begin----------')

    op = input('\n选择操作：\n1: 复制\n2: 还原\n\n')
    if op not in {'1', '2'}:
        raise ValueError('请选择 1 或 2')

    ignore_dirs = [
        '.git', '.vs', '.vscode', '.ide', 'build', 'CMake-build',
        'Depends', 'RDNet', 'x64', 'x86', 'EZUITools'
    ]

    if op == '1':
        print('\n' * 6)
        repo = input(f'\n操作仓库（位于 {source_dir}）：\n1: Upstream\n2: MantiSoft\n3: 自定义\n\n')
        if repo == '1':
            copy_dir = os.path.join(source_dir, 'Upstream')
        elif repo == '2':
            copy_dir = os.path.join(source_dir, 'MantiSoft')
        elif repo == '3':
            custom = input('自定义目录（空或 “.” 表示当前目录）：').strip()
            if not custom or custom == '.':
                copy_dir = os.getcwd()
                paster_dir = copy_dir + '_X'
            else:
                copy_dir = custom
                paster_dir = custom + '_X'
        else:
            raise ValueError('请选择 1, 2 或 3')

        if not os.path.exists(copy_dir):
            raise OSError(f'源路径不存在：{copy_dir}')

        HandleFile(copy_dir, paster_dir, ignore_dirs).copy_files()
    else:
        print('xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n')
        HandleFile(paster_dir, paster_dir, ignore_dirs).rnameSuffix()

    print('\n......The End......')