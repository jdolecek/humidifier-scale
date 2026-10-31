# Humidifier scale build gallery

[Project README](../README.md) · [Wiring diagram](two-channel-wiring.svg) · [ESPHome setup](../esphome/README.md)

The build uses two load-cell rails, one for each side of the humidifier. Each rail has two 20 kg load cells connected in parallel to one HX711. A Wemos D1 mini V4.0.0 reads the two HX711 channels separately and sends the readings to Home Assistant through ESPHome.

## Bench assembly

![Both load-cell rails on the workbench, connected to the D1 mini](photos/bench-assembly-overview.jpg)

*The two assembled rails before installation. Each has a pair of load cells and its own HX711 board; the jumper leads connect both channels to the D1 mini.*

![Close view of the load cells, mounting spacers, HX711 holders, and controller](photos/bench-load-cells-and-hx711.jpg)

*Close-up of the rail hardware, white mounting spacers and HX711 holders, bridge wiring, and USB-powered controller.*

## Installing the rails

![One load-cell rail positioned inside the overturned humidifier base](photos/single-rail-installation.jpg)

*One rail positioned in the overturned base. The original caster nearby shows the mounting arrangement before reassembly.*

![Wider view of the humidifier base during rail installation](photos/base-installation-in-progress.jpg)

*Installation in progress, with the original casters removed and one rail in place.*

## Installed supports and wiring

<img src="photos/installed-rails-and-casters.jpg" alt="Portrait view of both installed rails and their casters on the overturned humidifier" width="480">

*Both rails installed with the casters reattached. The white cables run along the underside of the base.*

![Side view showing the casters above the rail and load cells below it](photos/caster-and-load-cell-detail.jpg)

*Side view of the caster, rail, and load-cell arrangement. The humidifier is upside down in these installation photos.*

![Completed underside with both rails, four casters, and routed cables](photos/completed-underside-wiring.jpg)

*The completed underside: two instrumented rails, four casters, and cables secured along the base.*

For the electrical connections, use the [labeled wiring diagram](two-channel-wiring.svg). The photos document the physical build; wire colors alone do not identify a load cell's electrical functions.

## Separate temperature-probe photo

![Metal temperature probe with braided cable and two fork terminals](photos/separate-temperature-probe.jpg)

*An additional hardware photo supplied with this set: a metal temperature probe with braided cable and fork terminals. It is shown separately because it is not part of the HX711 tank-scale wiring.*

---

Photos supplied by the project owner. Documentation copies are resized for the web, stored upright, and omit the original camera/location metadata. The source photos are unchanged.
