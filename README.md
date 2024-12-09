# Contrôleur de Moteur à Courant Continu
## Description
Ce projet implémente un système de contrôle pour un moteur à courant continu utilisant un hacheur quatre quadrants. Le système est basé sur une carte STM32G431 et comprend une interface utilisateur via un shell UART pour contrôler le moteur et lire le courant moteur.

## Auteurs
- Constant Brissier
- Yacine Springer

## Caractéristiques
- Contrôle PWM d'un moteur à courant continu
- Interface utilisateur via shell UART
- Mesure de tension et de courant par ADC
- Ajustement progressif de la vitesse du moteur

## Matériel
- Carte de développement STM32G431
- Carte PCB personnalisée avec hacheur quatre quadrants
- Moteur à courant continu

## Fonctionnalités du Shell
| Commande | Description |
|----------|-------------|
| `help` | Affiche la liste des commandes disponibles |
| `pinout` | Affiche la liste des broches utilisées |
| `start` | Démarre le moteur |
| `stop` | Arrête le moteur |
| `speed <valeur>` | Ajuste la vitesse du moteur (0-100) |
| `adc` | Affiche les mesures actuelles de tension et de courant |

## Configuration
- UART2 utilisé pour la communication avec le shell
- Timer 1 configuré pour la génération PWM à 20 kHz
- ADC1 configuré pour la mesure de tension et de courant
- DMA utilisé pour les conversions ADC

## Utilisation
1. Connectez-vous au port série de la carte (115200 bauds)
2. Utilisez les commandes du shell pour contrôler le moteur et lire les mesures

## Notes
- Assurez-vous que toutes les connexions sont correctes avant de mettre sous tension
- La commande `speed` accepte des valeurs de 0 à 100, représentant le pourcentage de la vitesse maximale
- La commande `stop` fonctionne mais peut mener à des sauts de PWM quand start puis speed sont utilisés à la suite



