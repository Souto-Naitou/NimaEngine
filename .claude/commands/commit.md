---
description: gitの変更からコミットメッセージを生成してコミットする
allowed-tools: Bash(git status:*), Bash(git diff:*), Bash(git log:*), Bash(git add:*), Bash(git commit:*), Read
---

## 目的

現在のgitの変更内容を確認し、このリポジトリの慣習に沿ったコミットメッセージを生成してコミットする。

## コミットメッセージ形式

- 件名は `<種別>: <日本語の要約>` の形式。
- 種別は既存のコミット履歴に合わせる（例: `feat` / `fix` / `update` / `change` / `add` / `docs` / `clean/refactor`）。
- 必要なら空行を挟んで本文に変更理由・詳細を日本語で記述する。
- `Co-Authored-By` などの共同作成者トレーラーは付けない（Author はユーザー本人のみ）。

## 手順

1. `git status` と `git diff`（必要に応じて `--cached`）で変更内容を把握する。
2. `git log --oneline -20 --pretty=format:"%s"` で直近のコミット件名を確認し、種別・書き方の慣習に合わせる。
3. ステージされた変更が無ければ、関連する変更を `git add` でステージする（ユーザーが対象を指定していればそれに従う）。
4. 変更内容を要約したコミットメッセージを生成し、`git commit` する。
   - 日本語やマルチバイト文字を含むメッセージは、Bashツール（POSIX sh）では heredoc を使う（PowerShellの `@'...'@` は使わない）:

     ```bash
     git commit -F - <<'EOF'
     fix: 要約をここに書く

     本文をここに書く
     EOF
     ```
5. コミット後、`git log -1` で結果を確認し、要約をユーザーに報告する。

## 補足

- `$ARGUMENTS` が指定されている場合は、件名の方向性やステージ対象の指示として扱う。
- コミットのみ行い、push はユーザーが明示的に依頼した場合のみ行う。
