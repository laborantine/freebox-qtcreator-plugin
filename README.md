<div align="center">

# Freebox4-NG Ultimate

### Le SDK Freebox rencontre un Qt Creator plus moderne.

**Développer · Déboguer · Analyser · Tester · Automatiser**  
Une extension communautaire pour créer et diagnostiquer des applications QML sur **Freebox Révolution** et **Freebox Devialet**.

![Qt Creator](https://img.shields.io/badge/Qt_Creator-20.0.2-41CD52?style=flat-square&logo=qt&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-x64-0078D4?style=flat-square&logo=windows&logoColor=white)
![Version](https://img.shields.io/badge/Ultimate-v4-7557D9?style=flat-square)
![Statut](https://img.shields.io/badge/Statut-prototype_avancé-F59E0B?style=flat-square)

[**Découvrir les fonctionnalités**](#-ce-que-change-freebox4-ng) · [**Installer**](#-démarrage-rapide) · [**Roadmap**](#-feuille-de-route) · [**Changelog**](#-historique-des-versions)

</div>

---

> [!IMPORTANT]
> **Ultimate v4 est un prototype avancé**, ciblant **Qt Creator 20.0.2 / Windows x64**. La compilation, le chargement et plusieurs parcours ont été validés en environnement de test et sur Player simulé. **La validation complète sur Freebox Révolution et Devialet physiques reste à réaliser.**

## ✨ L'idée, en 30 secondes

Le plugin Freebox officiel permet déjà de travailler avec les projets Freebox depuis Qt Creator. **Freebox4-NG Ultimate** conserve cette base et pousse l'environnement plus loin :

| | Ce que nous cherchons à simplifier |
| :-- | :-- |
| 🛠️ **Développer** | Ouvrir un `.fbxproject`, vérifier le manifeste et préparer un `.fbxqml` |
| 🐞 **Déboguer** | Lancer, arrêter, inspecter le QML/JS et comprendre les problèmes de focus |
| 📊 **Diagnostiquer** | Rapprocher logs, événements réseau, télécommande et traces de performance |
| 🧪 **Tester** | Rejouer des scénarios et comparer les résultats entre deux versions |
| 🤖 **Automatiser** | Exposer des outils `freebox_*` à MCP, avec confirmations et accès limités |

**Notre ambition :** passer de *« ça ne marche pas sur ma Freebox »* à un problème **observable, reproductible et vérifiable**.

> Le projet est né de besoins concrets rencontrés sur [ReDeFin](https://github.com/laborantine/ReDeFin), un client Jellyfin pour Freebox. **Il n'est pas réservé à Jellyfin.**

## ⚡ Ce que change Freebox4-NG

### Le plugin officiel de Free vs notre extension

Référence : [**plugin Freebox officiel, branche Qt Creator 18**](https://github.com/fbx/freebox-qtcreator-plugin/tree/qtcreator-18.0.0). Cette branche annonce déjà **Run, Debug et Profiling**. Nous ne revendiquons donc pas ces fonctions comme des nouveautés exclusives.

| Fonction | Plugin officiel (Qt Creator 18) | Freebox4-NG Ultimate v4 |
| :-- | :--: | :-- |
| IDE ciblé | Qt Creator 18 | **Qt Creator 20.0.2** (Windows x64) |
| Projets `.fbxproject`, lancement, packaging | ✅ | ✅ Base conservée |
| Débogueur QML/JS et QML Profiler | ✅ | ✅ + outils de corrélation et comparaison |
| Inspection du focus | Débogage natif | **Panneau Live QML and Focus**, selon les données disponibles |
| Diagnostics orientés Freebox | Non documentés sous cette forme | **FBX_DIAG v3**, chronologie et exports |
| Télécommande et scénarios | Non documentés sous cette forme | **HID/RUDP**, enregistrement et rejeu JSON |
| Tests visuels | Non documentés sous cette forme | Comparaison de **captures fournies localement** |
| Intégration IA | Capacités propres à Qt Creator | **39 outils `freebox_*`** présents dans la v4 ; parcours agent à valider |

<sub>« Non documenté sous cette forme » ne signifie pas « impossible avec Qt Creator ». Les deux projets exploitent des capacités existantes de l'IDE ; Freebox4-NG développe surtout une couche spécialisée pour l'écosystème Freebox.</sub>

## 🧩 Dans la boîte à outils

<details open>
<summary><strong>🧑‍💻 Développement & déploiement</strong></summary>

- Projets `.fbxproject`, manifeste `manifest.json` et génération de paquets `.fbxqml`.
- Découverte de Players, choix de la cible et saisie manuelle d'une IP.
- Lancement via **Ctrl+R**, arrêt et amélioration du remplacement des sessions `main`.
- Vérifications ciblées des imports, ressources et incompatibilités connues avec le SDK Freebox.

</details>

<details open>
<summary><strong>🔍 QML, focus & performances</strong></summary>

- Débogage QML/JavaScript : points d'arrêt, variables, pile d'appels et pas à pas.
- Vue **Live QML and Focus** : inspection des informations accessibles, historique et export.
- Corrélation des événements de focus avec les touches de la télécommande.
- Analyse et comparaison de traces **QML Profiler `.qtd`** et de révisions Git.

**Limite :** l'observation de toutes les propriétés QML en temps réel n'est pas garantie. Certaines données nécessitent une instrumentation de l'application.

</details>

<details open>
<summary><strong>📡 Diagnostics & tests reproductibles</strong></summary>

- Chronologie **FBX_DIAG v3**, filtres et export anonymisé.
- Analyse d'événements QML/JS, HTTP, navigation, lecture multimédia et réseau, **lorsqu'ils sont disponibles**.
- Télécommande **HID/RUDP**, scénarios JSON rejouables, pauses et annulation.
- Assertions, comparaison de résultats et détection visuelle d'écarts sur des captures PNG/JPEG fournies.

**Limite :** le plugin ne capture pas encore automatiquement l'écran d'une Freebox physique et n'intercepte pas l'intégralité de son trafic réseau.

</details>

<details>
<summary><strong>🤖 MCP, Codex & assistance IA</strong></summary>

- **39 noms d'outils Freebox** présents dans la v4 pour piloter des opérations encadrées.
- Validation de projet, sélection de cible, lancement, consultation bornée des diagnostics, scénarios et inspection.
- Permissions par outil, confirmations locales et périmètre limité aux projets autorisés.
- Vision à terme : **diagnostiquer → proposer un patch → tester → comparer → demander validation**.

**État réel :** outils exercés avec un client de test ; intégration complète Codex/ACP **non encore validée**. L'IA n'est pas nécessaire pour utiliser les fonctions classiques du plugin.

</details>

## 🚀 Démarrage rapide

**Prérequis :** Qt Creator **20.0.2**, environnement **Windows x64**, DLL construite pour cette version précise, et Player Freebox configuré pour le développement.

1. **Fermer Qt Creator** et sauvegarder toute ancienne `Freebox4.dll`.
2. Copier la `Freebox4.dll` de la distribution Ultimate v4 dans le répertoire de plugins correspondant à **cette même installation** de Qt Creator.
3. Relancer Qt Creator et vérifier l'extension dans **Aide → À propos des extensions**.
4. Ouvrir un projet **`.fbxproject`**.
5. Sélectionner le Player via **Outils → Freebox SDK → Players and Active Target**, ou saisir son adresse IP.
6. Utiliser **Ctrl+R** pour lancer l'application ; **F5** pour déboguer.

> [!WARNING]
> Ne pas mélanger les DLL de versions différentes de Qt Creator. Prévoir une sauvegarde et un retour arrière. **La DLL v4 n'est pas présentée comme un binaire certifié pour tous les Players réels.**

<details>
<summary><strong>Compiler depuis les sources (environnement de maintenance v4)</strong></summary>

La chaîne documentée s'appuie sur **CMake, Visual Studio 2022 / MSVC x64** et un SDK Qt/Qt Creator compatible.

```powershell
.\scripts\Build-Windows-x64.ps1 `
  -QtPrefix C:\Qt\6.11.2\msvc2022_64 `
  -QtCreatorSdk C:\Qt\Creator20-sdk `
  -QtCreatorSource C:\Qt\Creator20-src
```

Ajouter `-BuildTests` lorsque le script et l'arborescence des sources correspondantes sont disponibles. Les chemins sont à adapter au poste de développement.

</details>

## 🧪 État du projet

| Domaine | État de validation |
| :-- | :-- |
| DLL v4 / Qt Creator 20.0.2 | ✅ Chargement dans un environnement **Windows x64 simulé** |
| Tests automatisés | ✅ **22 réussites Linux**, **20 réussites Windows simulé** (1 test ignoré) |
| Lancement, kit, découverte, remplacement de session | ✅ Parcours exercés sur **Player simulé** |
| Débogage QML F5 et comparaison `.qtd` | ✅ Exercés avec des fixtures de test |
| Endurance | ✅ **60 cycles**, **240 tentatives** avec Player simulé |
| Codex / ACP, bout en bout | ⚠️ À fiabiliser : énumération MCP expirée lors de l'essai Codex |
| Freebox Révolution / Devialet physiques | 🧪 **Validation complète en attente** |

Ces résultats décrivent **le périmètre testé**, pas une garantie générale de stabilité, de compatibilité matérielle ou d'absence de fuite mémoire.

## 🗺️ Feuille de route

- [ ] **Stabiliser la v4 sur matériel réel** : Révolution, Devialet, Ctrl+R, F5, Stop, reconnexions et HID.
- [ ] **Industrialiser les tests** : scénarios de référence, tests de non-régression et publications reproductibles.
- [ ] **Approfondir les diagnostics** : erreurs réseau/TLS, écrans noirs, focus, lecture Jellyfin et performance.
- [ ] **Améliorer l'inspection** : propriétés QML, objets, animations et comparaison des traces observables.
- [ ] **Finaliser la chaîne MCP / ACP / Codex** : découverte des outils, autorisations, sessions et résultats vérifiables.
- [ ] **Expérimenter la boucle IA supervisée** : correction isolée, test automatique, comparaison et validation humaine.

**Cap visé :** un environnement où chaque anomalie peut produire une trace, un scénario de reproduction et une preuve de correction.

## 📝 Historique des versions

<details>
<summary><strong>Ultimate v4</strong> · 13 novembre 2026 · prototype avancé</summary>

- Nouveau panneau **Live QML and Focus** et historique des transitions.
- Assertions enrichies, comparaisons de scénarios et résultats entre cibles.
- Comparaison de captures externes avec tolérances et masques.
- Lecture incrémentale des diagnostics via MCP ; **39 outils** maintenus.
- Mesures mémoire exploitables uniquement si présentes dans les traces.

</details>

<details>
<summary><strong>Ultimate v3</strong> · 13 novembre 2026</summary>

- Intégration de la télécommande **HID/RUDP**.
- Enregistrement, export et rejeu de scénarios JSON.
- Assertions liées aux commandes, au focus et à la chronologie.
- **10 outils MCP supplémentaires**, portant le total à **39**.

</details>

<details>
<summary><strong>Ultimate v2</strong> · novembre 2026</summary>

- Stabilisation des transitions **Stop / débogueur**.
- Inspection QML et diagnostics **FBX_DIAG v3**.
- Attentes MCP, comparaison de révisions Git et premiers essais d'endurance.

</details>

<details>
<summary><strong>Fondations Freebox4-NG</strong></summary>

- Adaptation du plugin natif à **Qt Creator 20.0.2**.
- Maintien des projets Freebox, de la découverte, du lancement et du packaging.
- Premières briques de validation, diagnostics et automatisation.

</details>

<sub>Cet historique retrace le développement du projet. Les versions listées ne constituent pas toutes des releases publiques certifiées sur les Players physiques.</sub>

## 🤝 Contribuer

Les signalements utiles précisent : **modèle de Player, firmware, version de Qt Creator, étapes de reproduction et résultat attendu/obtenu**. Ajoutez si possible une trace **anonymisée** ou un scénario minimal.

**Sécurité :** ne jamais publier de tokens Jellyfin, mots de passe, URL privées ou identifiants d'abonnés. Limiter MCP aux projets autorisés et ne pas exposer les ports de débogage à Internet.

## 📄 Crédits & licence

Freebox4-NG Ultimate est un **fork communautaire indépendant**, sans affiliation ni certification par Free ou Qt.

- **Base officielle :** [fbx/freebox-qtcreator-plugin](https://github.com/fbx/freebox-qtcreator-plugin)
- **Branche officielle de référence :** [`qtcreator-18.0.0`](https://github.com/fbx/freebox-qtcreator-plugin/tree/qtcreator-18.0.0)
- **SDK Freebox Player :** [documentation](https://dev.freebox.fr/sdk/player.html)
- **Cas d'usage :** [ReDeFin](https://github.com/laborantine/ReDeFin)
- **Modernisation :** Laborantine

Le plugin original est distribué sous **LGPL-2.1**. Conserver les notices de licence et d'attribution du code repris, ainsi que les licences propres aux éventuels composants tiers. Les marques Free, Freebox et Qt appartiennent à leurs titulaires respectifs.

---

<div align="center">

**Freebox4-NG Ultimate**  
*Coder sur le PC. Comprendre ce qui se passe sur la Freebox.*

</div>
