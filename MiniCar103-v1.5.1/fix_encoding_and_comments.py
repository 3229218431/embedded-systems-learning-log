#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
MiniCar103 项目编码修复和注释生成脚本
用途：
1. 检测文件编码并转换为UTF-8
2. 删除乱码注释
3. 使用Claude API生成详细中文注释

需要环境变量：
- CLAUDE_API_KEY: Claude API密钥

依赖库：
- chardet: pip install chardet
- anthropic: pip install anthropic
"""

import os
import sys
import re
import shutil
import chardet
import json
import logging
from pathlib import Path
from typing import List, Dict, Tuple, Optional
import anthropic

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('fix_encoding.log', encoding='utf-8'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

class CodeAnalyzer:
    """分析C代码结构"""

    @staticmethod
    def extract_functions(content: str) -> List[Dict]:
        """从C代码中提取函数定义"""
        functions = []

        # 匹配函数定义模式
        # 包括：返回类型 函数名(参数) { 或 返回类型 函数名(参数)\n{
        pattern = r'(\w+(?:\s+\*+)?)\s+(\w+)\s*\(([^)]*)\)\s*(?:{|$)'

        lines = content.split('\n')
        for i, line in enumerate(lines):
            # 跳过注释行
            stripped = line.strip()
            if stripped.startswith('//') or stripped.startswith('/*'):
                continue

            match = re.search(pattern, line)
            if match:
                return_type = match.group(1)
                func_name = match.group(2)
                params = match.group(3).strip()

                # 查找函数体开始
                body_start = i
                brace_count = 0
                found_brace = False

                for j in range(i, len(lines)):
                    if '{' in lines[j]:
                        if not found_brace:
                            body_start = j
                            found_brace = True
                        brace_count += lines[j].count('{')
                    if '}' in lines[j]:
                        brace_count -= lines[j].count('}')

                    if found_brace and brace_count == 0:
                        body_end = j
                        break
                else:
                    body_end = len(lines) - 1

                # 提取函数体
                func_body = '\n'.join(lines[body_start:body_end + 1])

                functions.append({
                    'name': func_name,
                    'return_type': return_type,
                    'params': params,
                    'line_start': i + 1,
                    'line_end': body_end + 1,
                    'body': func_body
                })

        return functions

    @staticmethod
    def extract_variables(content: str) -> List[Dict]:
        """提取全局变量和重要局部变量"""
        variables = []

        # 匹配全局变量定义
        patterns = [
            r'(?:const\s+)?(\w+(?:\s+\*+)?)\s+(\w+)\s*(?:=\s*[^;]+)?;',  # 简单变量
            r'(\w+(?:\s+\*+)?)\s+(\w+)\s*\[[^\]]*\];',  # 数组
            r'struct\s+(\w+)\s+(\w+)\s*;',  # 结构体变量
        ]

        for pattern in patterns:
            matches = re.finditer(pattern, content)
            for match in matches:
                var_type = match.group(1)
                var_name = match.group(2)

                # 跳过函数参数中的变量
                line = content[:match.start()].split('\n')[-1]
                if '(' in line and ')' in line:
                    continue

                variables.append({
                    'name': var_name,
                    'type': var_type,
                    'line': content[:match.start()].count('\n') + 1
                })

        return variables

class CommentGenerator:
    """使用Claude API生成注释"""

    def __init__(self, api_key: str):
        self.client = anthropic.Anthropic(api_key=api_key)

    def generate_file_header(self, file_path: str, content: str) -> str:
        """生成文件头注释"""
        prompt = f"""请为以下C源文件生成一个详细的中文文件头注释。该文件是STM32嵌入式项目的一部分。

文件路径：{file_path}

要求：
1. 使用标准的Doxygen格式
2. 包含文件描述、功能说明
3. 包含版权信息（使用原始版权信息：上海师范大学 2025-2035）
4. 包含注意事项
5. 使用中文

文件内容前200行：
```
{content[:5000]}
```

请只返回注释内容，不要额外解释。"""

        try:
            response = self.client.messages.create(
                model="claude-3-5-sonnet-20241022",
                max_tokens=1000,
                messages=[
                    {"role": "user", "content": prompt}
                ]
            )
            return response.content[0].text.strip()
        except Exception as e:
            logger.error(f"生成文件头注释失败: {e}")
            return ""

    def generate_function_comment(self, func_info: Dict) -> str:
        """生成函数注释"""
        prompt = f"""请为以下C函数生成详细的中文Doxygen风格注释。

函数原型：{func_info['return_type']} {func_info['name']}({func_info['params']})

函数体：
```c
{func_info['body']}
```

要求：
1. 使用标准的Doxygen格式（/** ... */）
2. 包含函数功能描述
3. 详细说明每个参数的含义
4. 说明返回值
5. 包含注意事项（如果有）
6. 使用中文

请只返回注释内容，不要额外解释。"""

        try:
            response = self.client.messages.create(
                model="claude-3-5-sonnet-20241022",
                max_tokens=800,
                messages=[
                    {"role": "user", "content": prompt}
                ]
            )
            return response.content[0].text.strip()
        except Exception as e:
            logger.error(f"生成函数注释失败 {func_info['name']}: {e}")
            return ""

    def generate_variable_comment(self, var_info: Dict) -> str:
        """生成变量注释"""
        prompt = f"""请为以下C变量生成简洁的中文注释。

变量声明：{var_info['type']} {var_info['name']}

要求：
1. 一行注释说明变量的用途
2. 使用中文
3. 如果是重要常量，说明其取值含义

请只返回注释内容，不要额外解释。"""

        try:
            response = self.client.messages.create(
                model="claude-3-5-sonnet-20241022",
                max_tokens=200,
                messages=[
                    {"role": "user", "content": prompt}
                ]
            )
            return response.content[0].text.strip()
        except Exception as e:
            logger.error(f"生成变量注释失败 {var_info['name']}: {e}")
            return ""

class EncodingFixer:
    """编码修复器"""

    def __init__(self, project_path: str, backup_path: str, api_key: str = None):
        self.project_path = Path(project_path)
        self.backup_path = Path(backup_path)
        self.comment_gen = CommentGenerator(api_key) if api_key else None

        # 文件扩展名过滤
        self.source_extensions = {'.c', '.h'}
        self.doc_extensions = {'.md', '.txt'}

        # 排除目录
        self.exclude_dirs = {
            'MiniCar103-backup',
            '.git',
            '__pycache__',
            'build',
            'Debug',
            'Release'
        }

    def backup_file(self, file_path: Path) -> bool:
        """备份文件"""
        try:
            relative_path = file_path.relative_to(self.project_path)
            backup_file_path = self.backup_path / relative_path
            backup_file_path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(file_path, backup_file_path)
            return True
        except Exception as e:
            logger.error(f"备份文件失败 {file_path}: {e}")
            return False

    def detect_encoding(self, file_path: Path) -> Tuple[str, float]:
        """检测文件编码"""
        try:
            with open(file_path, 'rb') as f:
                raw_data = f.read()

            result = chardet.detect(raw_data)
            encoding = result['encoding'] or 'utf-8'
            confidence = result['confidence'] or 0.0

            # 处理常见编码别名
            encoding_map = {
                'GB2312': 'gbk',
                'GBK': 'gbk',
                'GB18030': 'gb18030',
                'Windows-1252': 'cp1252',
                'ISO-8859-1': 'latin-1',
                'ascii': 'utf-8'
            }

            encoding = encoding_map.get(encoding.upper(), encoding).lower()

            return encoding, confidence
        except Exception as e:
            logger.error(f"检测编码失败 {file_path}: {e}")
            return 'utf-8', 0.0

    def convert_to_utf8(self, file_path: Path, encoding: str) -> bool:
        """将文件转换为UTF-8编码"""
        try:
            with open(file_path, 'r', encoding=encoding, errors='replace') as f:
                content = f.read()

            # 保存为UTF-8（无BOM）
            with open(file_path, 'w', encoding='utf-8', newline='') as f:
                f.write(content)

            return True
        except Exception as e:
            logger.error(f"转换编码失败 {file_path}: {e}")
            return False

    def is_garbled_text(self, text: str) -> bool:
        """检查文本是否包含乱码字符"""
        # 常见的乱码字符模式
        garbled_patterns = [
            r'�+',  # Unicode替换字符
            r'[\x80-\xff]{2,}',  # 连续的扩展ASCII字符
            r'[^\x00-\x7F\u4e00-\u9fa5\uff0c\u3002\uff1b\uff1a\u201c\u201d\uff08\uff09\u3001\uff1f\u300a\u300b]',  # 非中英文合法字符
        ]

        for pattern in garbled_patterns:
            if re.search(pattern, text):
                return True

        return False

    def remove_garbled_comments(self, content: str) -> str:
        """移除乱码注释"""
        lines = content.split('\n')
        cleaned_lines = []

        in_block_comment = False
        garbled_block = False

        for line in lines:
            # 处理块注释
            if '/*' in line and not in_block_comment:
                in_block_comment = True
                garbled_block = self.is_garbled_text(line)

            # 如果不在乱码块注释中，保留该行
            if not (in_block_comment and garbled_block):
                cleaned_lines.append(line)

            # 块注释结束
            if '*/' in line and in_block_comment:
                in_block_comment = False
                garbled_block = False

        return '\n'.join(cleaned_lines)

    def process_file(self, file_path: Path) -> Dict:
        """处理单个文件"""
        result = {
            'file': str(file_path),
            'backup_success': False,
            'encoding_detected': 'unknown',
            'encoding_confidence': 0.0,
            'converted': False,
            'comments_generated': False,
            'error': None
        }

        try:
            # 1. 备份文件
            result['backup_success'] = self.backup_file(file_path)

            # 2. 检测编码
            encoding, confidence = self.detect_encoding(file_path)
            result['encoding_detected'] = encoding
            result['encoding_confidence'] = confidence

            logger.info(f"检测文件编码: {file_path} -> {encoding} (置信度: {confidence:.2f})")

            # 3. 转换为UTF-8（如果不是UTF-8）
            if encoding not in ['utf-8', 'utf-8-sig'] and confidence > 0.5:
                if self.convert_to_utf8(file_path, encoding):
                    result['converted'] = True
                    logger.info(f"转换编码: {file_path} -> UTF-8")

            # 4. 读取内容
            with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
                content = f.read()

            # 5. 移除乱码注释
            cleaned_content = self.remove_garbled_comments(content)

            # 6. 对于源文件，生成新注释
            if file_path.suffix in self.source_extensions and self.comment_gen:
                # 分析代码结构
                analyzer = CodeAnalyzer()
                functions = analyzer.extract_functions(cleaned_content)
                variables = analyzer.extract_variables(cleaned_content)

                # 生成文件头注释
                file_header = self.comment_gen.generate_file_header(str(file_path), cleaned_content)

                # 生成函数注释
                func_comments = {}
                for func in functions[:10]:  # 限制数量避免API调用过多
                    comment = self.comment_gen.generate_function_comment(func)
                    if comment:
                        func_comments[func['name']] = comment

                # 生成变量注释（重要变量）
                var_comments = {}
                for var in variables[:20]:  # 限制数量
                    # 只注释看起来重要的变量（全局变量、常量等）
                    if not var['name'].startswith('_') and len(var['name']) > 1:
                        comment = self.comment_gen.generate_variable_comment(var)
                        if comment:
                            var_comments[var['name']] = comment

                if file_header or func_comments or var_comments:
                    result['comments_generated'] = True
                    logger.info(f"为 {file_path} 生成了 {len(func_comments)} 个函数注释和 {len(var_comments)} 个变量注释")

            # 7. 保存处理后的内容
            with open(file_path, 'w', encoding='utf-8', newline='') as f:
                f.write(cleaned_content)

        except Exception as e:
            result['error'] = str(e)
            logger.error(f"处理文件失败 {file_path}: {e}")

        return result

    def collect_files(self) -> List[Path]:
        """收集需要处理的文件"""
        files = []

        for ext in self.source_extensions | self.doc_extensions:
            for file_path in self.project_path.rglob(f'*{ext}'):
                # 跳过排除目录
                if any(exclude in file_path.parts for exclude in self.exclude_dirs):
                    continue

                # 跳过备份目录中的文件
                if self.backup_path in file_path.parents:
                    continue

                files.append(file_path)

        # 按优先级排序：用户代码 > 文档 > 库文件
        def get_priority(file_path: Path) -> int:
            path_str = str(file_path)
            if 'App' in path_str or 'Bsp' in path_str or 'Core' in path_str:
                return 1  # 最高优先级
            elif file_path.suffix in self.doc_extensions:
                return 2
            else:
                return 3  # 库文件

        files.sort(key=get_priority)
        return files

    def run(self):
        """运行处理流程"""
        # 收集文件
        files = self.collect_files()
        logger.info(f"找到 {len(files)} 个需要处理的文件")

        # 处理文件
        results = []
        for i, file_path in enumerate(files, 1):
            logger.info(f"处理文件 [{i}/{len(files)}]: {file_path}")
            result = self.process_file(file_path)
            results.append(result)

        # 生成报告
        self.generate_report(results)

    def generate_report(self, results: List[Dict]):
        """生成处理报告"""
        report_path = self.project_path / 'encoding_fix_report.md'

        processed_count = len(results)
        backup_success = sum(1 for r in results if r['backup_success'])
        converted_count = sum(1 for r in results if r['converted'])
        comments_generated = sum(1 for r in results if r['comments_generated'])
        error_count = sum(1 for r in results if r['error'])

        report_content = f"""# 编码修复和注释生成报告

## 统计信息
- 处理文件总数: {processed_count}
- 成功备份: {backup_success}
- 编码转换: {converted_count}
- 注释生成: {comments_generated}
- 错误数量: {error_count}

## 详细结果

| 文件 | 编码检测 | 置信度 | 转换 | 注释生成 | 错误 |
|------|----------|--------|------|----------|------|
"""

        for result in results:
            file_name = os.path.basename(result['file'])
            encoding = result['encoding_detected']
            confidence = result['encoding_confidence']
            converted = '✓' if result['converted'] else '✗'
            comments = '✓' if result['comments_generated'] else '✗'
            error = result['error'] or '-'

            report_content += f"| {file_name} | {encoding} | {confidence:.2f} | {converted} | {comments} | {error} |\n"

        with open(report_path, 'w', encoding='utf-8') as f:
            f.write(report_content)

        logger.info(f"报告已生成: {report_path}")

def main():
    """主函数"""
    # 检查环境变量
    api_key = os.environ.get('CLAUDE_API_KEY')
    if not api_key:
        logger.warning("未找到 CLAUDE_API_KEY 环境变量，将仅进行编码修复，不生成注释")
        api_key = None

    # 项目路径
    project_path = os.path.dirname(os.path.abspath(__file__))
    backup_path = os.path.join(os.path.dirname(project_path), 'MiniCar103-backup')

    # 检查依赖
    try:
        import chardet
    except ImportError:
        logger.error("缺少依赖库: chardet")
        logger.info("请运行: pip install chardet")
        sys.exit(1)

    if api_key:
        try:
            import anthropic
        except ImportError:
            logger.error("缺少依赖库: anthropic")
            logger.info("请运行: pip install anthropic")
            sys.exit(1)

    # 创建修复器并运行
    fixer = EncodingFixer(project_path, backup_path, api_key)
    fixer.run()

if __name__ == '__main__':
    main()