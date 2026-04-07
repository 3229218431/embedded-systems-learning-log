#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
MiniCar103 项目编码修复和注释生成脚本 v2
改进：
1. 更好的乱码检测和转换
2. 尝试多种编码修复乱码
3. 更详细的错误报告

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
from typing import List, Dict, Tuple, Optional, Set
import anthropic

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('fix_encoding_v2.log', encoding='utf-8'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

class EncodingFixerV2:
    """改进的编码修复器"""

    # 尝试的中文编码列表（按优先级）
    CHINESE_ENCODINGS = ['gbk', 'gb2312', 'gb18030', 'cp936', 'big5', 'utf-8']

    def __init__(self, project_path: str, backup_path: str, api_key: str = None):
        self.project_path = Path(project_path)
        self.backup_path = Path(backup_path)
        self.api_key = api_key

        # 文件扩展名过滤
        self.source_extensions = {'.c', '.h'}
        self.doc_extensions = {'.md', '.txt', '.md'}

        # 排除目录
        self.exclude_dirs = {
            'MiniCar103-backup',
            '.git',
            '__pycache__',
            'build',
            'Debug',
            'Release'
        }

        # 重要目录（优先处理）
        self.important_dirs = {'App', 'Bsp', 'Core'}

    def is_likely_chinese_file(self, file_path: Path) -> bool:
        """判断文件是否可能包含中文内容"""
        # 检查路径是否在重要目录中
        for dir_name in self.important_dirs:
            if dir_name in str(file_path):
                return True

        # 检查文件名
        important_files = {'README', 'CLAUDE', 'LICENSE'}
        for name in important_files:
            if name in file_path.name:
                return True

        return False

    def detect_encoding_advanced(self, file_path: Path) -> Tuple[str, float, bool]:
        """高级编码检测，特别处理中文乱码"""
        try:
            with open(file_path, 'rb') as f:
                raw_data = f.read()

            # 首先用chardet检测
            result = chardet.detect(raw_data)
            detected_encoding = result['encoding'] or 'utf-8'
            confidence = result['confidence'] or 0.0

            # 标准化编码名称
            encoding_map = {
                'GB2312': 'gbk',
                'GBK': 'gbk',
                'GB18030': 'gb18030',
                'Windows-1252': 'cp1252',
                'ISO-8859-1': 'latin-1',
                'ascii': 'utf-8',
                'UTF-8-SIG': 'utf-8'
            }

            detected_encoding = encoding_map.get(detected_encoding.upper(), detected_encoding).lower()

            # 如果检测到UTF-8但文件可能包含中文，检查是否乱码
            if detected_encoding == 'utf-8' and confidence > 0.7:
                try:
                    # 尝试用UTF-8解码
                    decoded = raw_data.decode('utf-8', errors='strict')
                    # 检查是否包含乱码字符
                    if self.contains_garbled_chinese(decoded):
                        # 可能是GBK编码被错误地标记为UTF-8
                        logger.info(f"文件 {file_path} 标记为UTF-8但包含乱码，尝试其他编码")
                        return detected_encoding, confidence, True
                except UnicodeDecodeError:
                    # UTF-8解码失败，肯定不是UTF-8
                    pass

            return detected_encoding, confidence, False

        except Exception as e:
            logger.error(f"高级编码检测失败 {file_path}: {e}")
            return 'utf-8', 0.0, False

    def contains_garbled_chinese(self, text: str) -> bool:
        """检查文本是否包含中文乱码"""
        # 常见的乱码模式
        garbled_patterns = [
            r'�+',  # Unicode替换字符
            r'[\x80-\xff]{2,}',  # 连续的扩展ASCII字符
        ]

        for pattern in garbled_patterns:
            if re.search(pattern, text):
                return True

        # 检查是否包含可能的中文字符但结构异常
        # 中文字符范围：\u4e00-\u9fff
        chinese_chars = re.findall(r'[\u4e00-\u9fff]', text)
        if len(chinese_chars) > 0:
            # 如果有中文字符但也有很多乱码字符，可能是乱码
            garbled_chars = re.findall(r'[^\x00-\x7F\u4e00-\u9fff\uff0c\u3002\uff1b\uff1a\u201c\u201d\uff08\uff09\u3001\uff1f\u300a\u300b\s]', text)
            if len(garbled_chars) > len(chinese_chars) / 2:
                return True

        return False

    def try_decode_with_encodings(self, raw_data: bytes) -> Tuple[Optional[str], Optional[str]]:
        """尝试用多种编码解码数据"""
        for encoding in self.CHINESE_ENCODINGS:
            try:
                decoded = raw_data.decode(encoding, errors='strict')
                # 检查解码后的文本是否合理
                if not self.contains_garbled_chinese(decoded):
                    return decoded, encoding
            except UnicodeDecodeError:
                continue

        # 如果所有编码都失败，尝试用errors='replace'
        for encoding in self.CHINESE_ENCODINGS:
            try:
                decoded = raw_data.decode(encoding, errors='replace')
                return decoded, encoding
            except:
                continue

        return None, None

    def fix_garbled_content(self, file_path: Path) -> bool:
        """修复乱码内容"""
        try:
            with open(file_path, 'rb') as f:
                raw_data = f.read()

            # 尝试用多种编码解码
            decoded_content, used_encoding = self.try_decode_with_encodings(raw_data)

            if decoded_content is None:
                logger.error(f"无法解码文件 {file_path}")
                return False

            logger.info(f"文件 {file_path} 使用编码 {used_encoding} 解码成功")

            # 保存为UTF-8
            with open(file_path, 'w', encoding='utf-8', newline='') as f:
                f.write(decoded_content)

            return True

        except Exception as e:
            logger.error(f"修复乱码内容失败 {file_path}: {e}")
            return False

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

    def process_file(self, file_path: Path) -> Dict:
        """处理单个文件"""
        result = {
            'file': str(file_path),
            'backup_success': False,
            'encoding_detected': 'unknown',
            'encoding_confidence': 0.0,
            'needs_fix': False,
            'fixed': False,
            'error': None
        }

        try:
            # 1. 备份文件
            result['backup_success'] = self.backup_file(file_path)

            # 2. 检测编码和是否需要修复
            encoding, confidence, needs_fix = self.detect_encoding_advanced(file_path)
            result['encoding_detected'] = encoding
            result['encoding_confidence'] = confidence
            result['needs_fix'] = needs_fix

            logger.info(f"文件 {file_path} -> {encoding} (置信度: {confidence:.2f}, 需要修复: {needs_fix})")

            # 3. 如果需要修复，尝试修复乱码
            if needs_fix or self.is_likely_chinese_file(file_path):
                if self.fix_garbled_content(file_path):
                    result['fixed'] = True
                    logger.info(f"修复乱码: {file_path}")

            # 4. 确保文件是UTF-8编码
            with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
                content = f.read()

            # 5. 移除明显的乱码注释块
            cleaned_content = self.remove_garbled_comments(content)

            if cleaned_content != content:
                with open(file_path, 'w', encoding='utf-8', newline='') as f:
                    f.write(cleaned_content)
                logger.info(f"清理乱码注释: {file_path}")

        except Exception as e:
            result['error'] = str(e)
            logger.error(f"处理文件失败 {file_path}: {e}")

        return result

    def remove_garbled_comments(self, content: str) -> str:
        """移除乱码注释"""
        lines = content.split('\n')
        cleaned_lines = []

        in_block_comment = False
        garbled_block = False

        for line in lines:
            # 处理块注释开始
            if '/*' in line and not in_block_comment:
                in_block_comment = True
                # 检查是否乱码
                garbled_block = self.contains_garbled_chinese(line)

            # 如果不在乱码块注释中，保留该行
            if not (in_block_comment and garbled_block):
                cleaned_lines.append(line)

            # 处理块注释结束
            if '*/' in line and in_block_comment:
                in_block_comment = False
                garbled_block = False

        return '\n'.join(cleaned_lines)

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

        # 按优先级排序：重要目录 > 文档 > 其他
        def get_priority(file_path: Path) -> int:
            path_str = str(file_path)
            if any(dir_name in path_str for dir_name in self.important_dirs):
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
            if i <= 10 or file_path.suffix in self.doc_extensions or self.is_likely_chinese_file(file_path):
                logger.info(f"处理文件 [{i}/{len(files)}]: {file_path}")
                result = self.process_file(file_path)
                results.append(result)
            else:
                # 对于库文件，只记录不处理
                results.append({
                    'file': str(file_path),
                    'backup_success': False,
                    'encoding_detected': 'skipped',
                    'encoding_confidence': 0.0,
                    'needs_fix': False,
                    'fixed': False,
                    'error': 'skipped (library file)'
                })

        # 生成报告
        self.generate_report(results)

    def generate_report(self, results: List[Dict]):
        """生成处理报告"""
        report_path = self.project_path / 'encoding_fix_report_v2.md'

        processed_count = len(results)
        backup_success = sum(1 for r in results if r['backup_success'])
        needs_fix_count = sum(1 for r in results if r['needs_fix'])
        fixed_count = sum(1 for r in results if r['fixed'])
        error_count = sum(1 for r in results if r['error'] and r['error'] != 'skipped (library file)')
        skipped_count = sum(1 for r in results if r['error'] == 'skipped (library file)')

        report_content = f"""# 编码修复报告 v2

## 统计信息
- 扫描文件总数: {processed_count}
- 成功备份: {backup_success}
- 需要修复: {needs_fix_count}
- 已修复: {fixed_count}
- 错误数量: {error_count}
- 跳过库文件: {skipped_count}

## 详细结果

| 文件 | 编码检测 | 置信度 | 需要修复 | 已修复 | 错误 |
|------|----------|--------|----------|--------|------|
"""

        for result in results:
            file_name = os.path.basename(result['file'])
            encoding = result['encoding_detected']
            confidence = result['encoding_confidence']
            needs_fix = '✓' if result['needs_fix'] else '✗'
            fixed = '✓' if result['fixed'] else '✗'
            error = result['error'] or '-'

            report_content += f"| {file_name} | {encoding} | {confidence:.2f} | {needs_fix} | {fixed} | {error} |\n"

        with open(report_path, 'w', encoding='utf-8') as f:
            f.write(report_content)

        logger.info(f"报告已生成: {report_path}")

def main():
    """主函数"""
    # 项目路径
    project_path = os.path.dirname(os.path.abspath(__file__))
    backup_path = os.path.join(os.path.dirname(project_path), 'MiniCar103-backup-v2')

    # 检查依赖
    try:
        import chardet
    except ImportError:
        logger.error("缺少依赖库: chardet")
        logger.info("请运行: pip install chardet")
        sys.exit(1)

    # 创建修复器并运行
    fixer = EncodingFixerV2(project_path, backup_path, api_key=None)
    fixer.run()

    print("\n" + "="*60)
    print("处理完成！")
    print("="*60)
    print("\n接下来：")
    print("1. 检查 encoding_fix_report_v2.md 查看处理结果")
    print("2. 检查关键文件是否已修复乱码")
    print("3. 如果需要生成注释，请设置 CLAUDE_API_KEY 环境变量")
    print("4. 运行注释生成脚本")

if __name__ == '__main__':
    main()