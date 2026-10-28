# GitHub release checklist

1. Create an empty GitHub repository named `VoxRageVST3`.
2. Copy this project into it.
3. Commit and push:
   ```bash
   git init
   git add .
   git commit -m "Initial VoxRage VST3"
   git branch -M main
   git remote add origin https://github.com/YOUR_USERNAME/VoxRageVST3.git
   git push -u origin main
   ```
4. Open **Actions** on GitHub. The macOS workflow builds a universal VST3/AU/Standalone artifact.
5. Download the workflow artifact `VoxRage-macOS`.

For public distribution, add your own signing/notarization secrets and release workflow.
