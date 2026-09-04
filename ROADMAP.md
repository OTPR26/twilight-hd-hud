# Roadmap

## Planned

- Harden the self-updater by validating the published SHA-256 digest and
  `org.twilight.hd_hud` manifest identity before installation, and add a
  release-page fallback on platforms without a supported download transport.
- Normalize HUD scaling across platforms and display densities so the same size
  setting has a consistent visual footprint on Android, macOS, Windows, iOS,
  and tvOS. Android at 125% currently appears close to desktop at 100%.
