# TODO

## Done

- [x] Rename the executable target to `mdp-generator`
- [x] Update the desktop launcher to use `Exec=mdp-generator`
- [x] Keep `mdp-logo.png` as the application icon
- [x] Add an `install.sh` helper script
- [x] Refresh the README so it matches the current command name and installation flow

## Next

- [x] Rework the random generation flow so microphone noise is the primary entropy source
- [x] Add automated tests and measure coverage
- [x] Target around 95% coverage on the non-UI logic
- [x] Launch and validate the GTK UI manually on Ubuntu
- [x] Split the work into several clean commits

## Blockers

- [ ] Open the PR from an environment with a working GitHub CLI or integration token
- [ ] Authenticate `gh` to open the PR from this environment
