# InverterLayerCompat

Compatibility `Layer` that can be used where `InverterLayer` is not available
(basically everywhere).

```c
static InverterLayerCompat *s_inverter_layer;
```

```c
s_inverter_layer = inverter_layer_compat_create(bounds, GColorBlack, GColorWhite);
layer_add_child(window_layer, inverter_layer_compat_get_layer(s_inverter_layer));
```

```c
inverter_layer_compat_destroy(s_inverter_layer);
```
