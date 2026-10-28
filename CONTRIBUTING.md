# Contributing

## Branch workflow

Development is organized in separate team and lab branches.

- `main` - original course template
- `Blobick` - team branch
- `Blobick_wolf` - Raycaster project branch
- `Blobick_wolf_lab01` - Lab 1 development

Do not commit directly to `main`.

Keep commits focused on a single change.

## Commit messages

This project follows Conventional Commits.

Format:

`type(scope): short description`

Common types:

- `feat` - new functionality
- `fix` - bug fix
- `docs` - documentation
- `test` - tests
- `refactor` - code restructuring
- `build` - build configuration
- `ci` - continuous integration
- `chore` - repository maintenance

Examples:

- `feat(world): add map loading`
- `feat(raycast): implement DDA traversal`
- `fix(render): correct wall column clipping`
- `test(render): cover framebuffer edge cases`
- `docs(repo): update build instructions`

Use short, descriptive commit messages.

## Code organization

Keep modules independent where possible.

- `world/` - map and player state
- `raycast/` - ray traversal and projection
- `render/` - CPU framebuffer and SDL presentation

SDL should not be used for wall geometry calculations.

Public interfaces should be agreed upon before changing
them in ways that affect other modules.

## Before committing

- Build the project.
- Run available tests.
- Check compiler warnings.
- Avoid committing generated files.
- Keep changes limited to the relevant module.

Build instructions are available in [README.md](README.md).
