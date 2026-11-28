// ARMOR-NODE-SOLAR-HMI - the words of the screen, in the eight languages of the ecosystem.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// One table: a key and its eight texts (en, es, de, fr, it, ja, zh, pt). A key without a row is drawn as the key itself, and the tests fail when a row has an empty text, so a text
// can never reach the screen in only some of the languages. The Latin languages are drawn with the Latin font; Japanese and Chinese have a font of their own with only the glyphs
// these texts use (tools/make_fonts.py makes them, tests/check_font_glyphs.py checks that every character of this table and of main/ui.cpp has its glyph).
#pragma once
#include <array>
#include <cstddef>
#include <string_view>

namespace armor::solarhmi::text {

constexpr std::array<std::string_view, 8> kLanguages{"en", "es", "de", "fr", "it", "ja", "zh", "pt"};

struct Row {
  std::string_view key;
  std::array<std::string_view, 8> text;   // in the order of kLanguages
};

// clang-format off
constexpr Row kRows[] = {
    {"nav_summary", {"Summary", "Resumen", "Übersicht", "Résumé", "Riepilogo", "概要", "概览", "Resumo"}},
    {"nav_batteries", {"Batteries", "Baterías", "Batterien", "Batteries", "Batterie", "電池", "电池", "Baterias"}},
    {"nav_inverters", {"Inverters", "Inversores", "Wechselrichter", "Onduleurs", "Inverter", "インバーター", "逆变器", "Inversores"}},
    {"nav_alarms", {"Alarms", "Alarmas", "Alarme", "Alarmes", "Allarmi", "警報", "警报", "Alarmes"}},
    {"nav_settings", {"Settings", "Ajustes", "Einstellungen", "Réglages", "Impostazioni", "設定", "设置", "Definições"}},
    {"link_online", {"Connected", "Conectado", "Verbunden", "Connecté", "Collegato", "接続中", "已连接", "Ligado"}},
    {"link_stale", {"No answer", "Sin respuesta", "Keine Antwort", "Sans réponse", "Nessuna risposta", "応答なし", "无应答", "Sem resposta"}},
    {"link_never", {"Not set up", "Sin configurar", "Nicht eingerichtet", "Non configuré", "Non configurato", "未設定", "尚未设置", "Por configurar"}},
    {"old_data", {"Old data: read only", "Datos antiguos: solo lectura", "Alte Daten: nur lesen", "Données anciennes : lecture seule", "Dati vecchi: sola lettura", "古いデータ：読み取りのみ", "数据过旧：只读", "Dados antigos: só leitura"}},
    {"waiting", {"Waiting for the server", "Esperando al servidor", "Warten auf den Server", "En attente du serveur", "In attesa del server", "サーバーを待っています", "正在等待服务器", "A aguardar o servidor"}},
    {"no_devices", {"No solar equipment yet", "Aún no hay equipos solares", "Noch keine Solargeräte", "Aucun équipement solaire", "Ancora nessun apparecchio", "太陽光機器はまだありません", "尚无太阳能设备", "Ainda sem equipamentos solares"}},
    {"not_shown", {"Some equipment is not shown", "Algún equipo no se muestra", "Nicht alle Geräte werden angezeigt", "Certains équipements ne sont pas affichés", "Alcuni apparecchi non sono mostrati", "表示されない機器があります", "部分设备未显示", "Alguns equipamentos não são mostrados"}},
    {"soc", {"State of charge", "Estado de carga", "Ladezustand", "État de charge", "Stato di carica", "充電率", "荷电状态", "Estado de carga"}},
    {"solar_power", {"Solar panels", "Paneles solares", "Solarmodule", "Panneaux solaires", "Pannelli solari", "ソーラーパネル", "太阳能板", "Painéis solares"}},
    {"energy_today", {"today", "hoy", "heute", "aujourd'hui", "oggi", "今日", "今天", "hoje"}},
    {"energy_total", {"in all", "en total", "insgesamt", "au total", "in totale", "累計", "累计", "no total"}},
    {"stack_health", {"Health of the stack", "Salud de la pila", "Zustand des Stapels", "Santé de la pile", "Salute del pacco", "スタックの劣化状態", "电池组健康度", "Saúde do conjunto"}},
    {"load_power", {"Consumption", "Consumo", "Verbrauch", "Consommation", "Consumo", "消費", "用电", "Consumo"}},
    {"battery_power", {"Battery", "Batería", "Batterie", "Batterie", "Batteria", "バッテリー", "电池", "Bateria"}},
    {"stored_energy", {"Stored energy", "Energía guardada", "Gespeicherte Energie", "Énergie stockée", "Energia immagazzinata", "蓄電量", "储存电量", "Energia armazenada"}},
    {"charging", {"charging", "cargando", "lädt", "en charge", "in carica", "充電中", "充电中", "a carregar"}},
    {"discharging", {"discharging", "descargando", "entlädt", "en décharge", "in scarica", "放電中", "放电中", "a descarregar"}},
    {"idle", {"idle", "en reposo", "ruht", "au repos", "a riposo", "待機", "静置", "em repouso"}},
    {"stack", {"Stack", "Conjunto", "Stapel", "Pile", "Pacco", "スタック", "电池组", "Conjunto"}},
    {"module", {"Battery", "Batería", "Batterie", "Batterie", "Batteria", "電池", "电池", "Bateria"}},
    {"cells", {"Cells", "Celdas", "Zellen", "Cellules", "Celle", "セル", "电芯", "Células"}},
    {"no_cells", {"This battery does not say its cells", "Esta batería no dice sus celdas", "Diese Batterie meldet keine Zellen", "Cette batterie ne donne pas ses cellules", "Questa batteria non indica le celle", "この電池はセル値を返しません", "该电池未提供电芯数据", "Esta bateria não indica as células"}},
    {"min", {"Min", "Mín", "Min", "Min", "Min", "最小", "最小", "Mín"}},
    {"max", {"Max", "Máx", "Max", "Max", "Max", "最大", "最大", "Máx"}},
    {"spread", {"Spread", "Diferencia", "Differenz", "Écart", "Scarto", "差", "压差", "Diferença"}},
    {"temperature", {"Temperature", "Temperatura", "Temperatur", "Température", "Temperatura", "温度", "温度", "Temperatura"}},
    {"voltage", {"Voltage", "Tensión", "Spannung", "Tension", "Tensione", "電圧", "电压", "Tensão"}},
    {"current", {"Current", "Corriente", "Strom", "Courant", "Corrente", "電流", "电流", "Corrente"}},
    {"level_ok", {"OK", "Bien", "OK", "OK", "OK", "正常", "正常", "Bem"}},
    {"level_warning", {"Warning", "Aviso", "Warnung", "Attention", "Avviso", "注意", "注意", "Aviso"}},
    {"level_alarm", {"Alarm", "Alarma", "Alarm", "Alarme", "Allarme", "警報", "警报", "Alarme"}},
    {"level_fault", {"Fault", "Fallo", "Defekt", "Panne", "Guasto", "故障", "故障", "Avaria"}},
    {"bms_fault", {"Battery electronics fault (BMS)", "Fallo de la electrónica (BMS)", "Fehler der Elektronik (BMS)", "Panne de l'électronique (BMS)", "Guasto dell'elettronica (BMS)", "電池回路の故障 (BMS)", "电池电路故障 (BMS)", "Falha da eletrónica (BMS)"}},
    {"bms_protect", {"The battery protects itself", "La batería se protege", "Die Batterie schützt sich", "La batterie se protège", "La batteria si protegge", "電池が保護動作中", "电池保护中", "A bateria protege-se"}},
    {"bms_notice", {"Battery notice", "Aviso de la batería", "Hinweis der Batterie", "Avis de la batterie", "Avviso della batteria", "電池の注意", "电池提示", "Aviso da bateria"}},
    {"left_out", {"Impossible figures left out", "Cifras imposibles omitidas", "Unmögliche Werte ausgelassen", "Valeurs impossibles ignorées", "Valori impossibili omessi", "あり得ない値を除外", "已略去不可能的数值", "Valores impossíveis omitidos"}},
    {"deduced", {"deduced", "deducido", "abgeleitet", "déduit", "dedotto", "推定", "推断", "deduzido"}},
    {"mode", {"Mode", "Modo", "Betriebsart", "Mode", "Modo", "モード", "模式", "Modo"}},
    {"grid", {"Grid", "Red", "Netz", "Réseau", "Rete", "系統", "电网", "Rede"}},
    {"output", {"Output", "Salida", "Ausgang", "Sortie", "Uscita", "出力", "输出", "Saída"}},
    {"pv", {"Panels", "Paneles", "Module", "Panneaux", "Pannelli", "パネル", "光伏", "Painéis"}},
    {"heatsink", {"Heatsink", "Disipador", "Kühlkörper", "Dissipateur", "Dissipatore", "放熱板", "散热器", "Dissipador"}},
    {"load_percent", {"Load", "Carga", "Last", "Charge", "Carico", "負荷", "负载", "Carga"}},
    {"warnings", {"Warnings", "Avisos", "Warnungen", "Avertissements", "Avvisi", "注意事項", "警告", "Avisos"}},
    {"none", {"None", "Ninguno", "Keine", "Aucun", "Nessuno", "なし", "无", "Nenhum"}},
    {"mode_power_on", {"Starting", "Arrancando", "Startet", "Démarrage", "Avvio", "起動中", "启动中", "A arrancar"}},
    {"mode_standby", {"Standby", "En espera", "Bereitschaft", "Veille", "Attesa", "スタンバイ", "待机", "Em espera"}},
    {"mode_line", {"Grid", "Con red", "Netzbetrieb", "Sur réseau", "Da rete", "系統運転", "市电模式", "Com rede"}},
    {"mode_battery", {"Battery", "Con batería", "Batteriebetrieb", "Sur batterie", "Da batteria", "電池運転", "电池模式", "Com bateria"}},
    {"mode_fault", {"Fault", "Fallo", "Störung", "Défaut", "Guasto", "故障", "故障", "Falha"}},
    {"mode_power_saving", {"Power saving", "Ahorro", "Sparbetrieb", "Économie", "Risparmio", "省電力", "节能", "Poupança"}},
    {"mode_shutdown", {"Off", "Apagado", "Aus", "Arrêté", "Spento", "停止", "已关闭", "Desligado"}},
    {"mode_unknown", {"Unknown", "Desconocido", "Unbekannt", "Inconnu", "Sconosciuto", "不明", "未知", "Desconhecido"}},
    {"cell_high", {"Cell voltage too high", "Tensión de celda alta", "Zellspannung zu hoch", "Tension de cellule trop haute", "Tensione di cella alta", "セル電圧が高すぎます", "电芯电压过高", "Tensão de célula alta"}},
    {"cell_low", {"Cell voltage too low", "Tensión de celda baja", "Zellspannung zu niedrig", "Tension de cellule trop basse", "Tensione di cella bassa", "セル電圧が低すぎます", "电芯电压过低", "Tensão de célula baixa"}},
    {"spread_high", {"Cells out of balance", "Celdas desequilibradas", "Zellen unausgeglichen", "Cellules déséquilibrées", "Celle sbilanciate", "セルの偏り", "电芯不均衡", "Células desequilibradas"}},
    {"temp_out", {"Temperature out of range", "Temperatura fuera de margen", "Temperatur außer Bereich", "Température hors limites", "Temperatura fuori limite", "温度が範囲外", "温度超出范围", "Temperatura fora dos limites"}},
    {"charge_low", {"Low charge", "Carga baja", "Ladung niedrig", "Charge faible", "Carica bassa", "残量低下", "电量低", "Carga baixa"}},
    {"device_old", {"The equipment is silent", "El equipo no responde", "Das Gerät schweigt", "L'équipement ne répond pas", "L'apparecchio tace", "機器の応答がありません", "设备无响应", "O equipamento não responde"}},
    {"battery_alarm", {"The battery reports an alarm", "La batería indica alarma", "Die Batterie meldet Alarm", "La batterie signale une alarme", "La batteria segnala un allarme", "電池が警報を出しています", "电池报告警报", "A bateria indica alarme"}},
    {"module_missing", {"Battery missing", "Falta una batería", "Batterie fehlt", "Batterie absente", "Batteria assente", "電池がありません", "电池缺失", "Falta uma bateria"}},
    {"inverter_warning", {"Inverter warning", "Aviso del inversor", "Warnung des Wechselrichters", "Avertissement onduleur", "Avviso dell'inverter", "インバーターの注意", "逆变器警告", "Aviso do inversor"}},
    {"inverter_fault", {"Inverter fault", "Fallo del inversor", "Störung des Wechselrichters", "Défaut onduleur", "Guasto dell'inverter", "インバーター故障", "逆变器故障", "Falha do inversor"}},
    {"all_quiet", {"All quiet", "Todo en calma", "Alles ruhig", "Tout est calme", "Tutto tranquillo", "異常なし", "一切正常", "Tudo calmo"}},
    {"check", {"Check again", "Verificar", "Neu prüfen", "Vérifier", "Verifica", "再確認", "复查", "Verificar"}},
    {"reset_counters", {"Reset counters", "Borrar contadores", "Zähler löschen", "Effacer les compteurs", "Azzera contatori", "カウンタ消去", "清零计数器", "Apagar contadores"}},
    {"back", {"Back", "Volver", "Zurück", "Retour", "Indietro", "戻る", "返回", "Voltar"}},
    {"pin_title", {"Type the PIN", "Escribe el PIN", "PIN eingeben", "Saisissez le code", "Digita il PIN", "PIN を入力", "请输入 PIN", "Escreva o PIN"}},
    {"pin_wrong", {"The PIN is not right", "El PIN no es correcto", "Die PIN ist falsch", "Le code n'est pas bon", "Il PIN non è corretto", "PIN が違います", "PIN 不正确", "O PIN não está correto"}},
    {"pin_locked", {"Too many wrong tries: wait", "Demasiados intentos: espera", "Zu viele Fehlversuche: warten", "Trop d'essais : patientez", "Troppi tentativi: attendi", "失敗が多いため待機してください", "错误次数过多：请等待", "Demasiadas tentativas: aguarde"}},
    {"ok", {"OK", "Aceptar", "OK", "Valider", "Conferma", "決定", "确定", "Confirmar"}},
    {"clear", {"Clear", "Borrar", "Löschen", "Effacer", "Cancella", "消去", "清除", "Apagar"}},
    {"cancel", {"Cancel", "Cancelar", "Abbrechen", "Annuler", "Annulla", "中止", "取消", "Cancelar"}},
    {"yes", {"Yes", "Sí", "Ja", "Oui", "Sì", "はい", "是", "Sim"}},
    {"no", {"No", "No", "Nein", "Non", "No", "いいえ", "否", "Não"}},
    {"confirm_check", {"Read this battery again?", "¿Volver a leer esta batería?", "Diese Batterie neu lesen?", "Relire cette batterie ?", "Rileggere questa batteria?", "この電池を読み直しますか？", "重新读取此电池？", "Ler outra vez esta bateria?"}},
    {"confirm_reset", {"This erases ALL the counters of this battery. They cannot be brought back.", "Esto borra TODOS los contadores de esta batería. No se pueden recuperar.", "Dies löscht ALLE Zähler dieser Batterie. Sie lassen sich nicht wiederherstellen.", "Ceci efface TOUS les compteurs de cette batterie. Ils ne pourront pas être récupérés.", "Questo cancella TUTTI i contatori della batteria. Non si possono recuperare.", "この電池のすべてのカウンタを消去します。元に戻せません。", "将清除此电池的全部计数器，且无法恢复。", "Isto apaga TODOS os contadores desta bateria. Não podem ser recuperados."}},
    {"block_not_from_screen", {"Not possible from this screen", "No se puede desde esta pantalla", "Von diesem Bildschirm nicht möglich", "Impossible depuis cet écran", "Non possibile da questo schermo", "この画面では実行できません", "无法在本屏幕执行", "Não é possível neste ecrã"}},
    {"block_server_off", {"Battery operations are off in the server", "El servidor no permite operar con baterías", "Batterieaktionen sind im Server aus", "Opérations batterie désactivées sur le serveur", "Operazioni sulle batterie spente nel server", "サーバーで電池操作が無効です", "服务器未开启电池操作", "O servidor não permite operar baterias"}},
    {"block_no_permission", {"This account may not do it", "Esta cuenta no puede hacerlo", "Dieses Konto darf das nicht", "Ce compte n'y est pas autorisé", "Questo account non può farlo", "このアカウントには権限がありません", "此账户无权执行", "Esta conta não pode fazê-lo"}},
    {"block_no_server", {"No fresh answer from the server", "El servidor no responde ahora", "Keine aktuelle Antwort vom Server", "Pas de réponse récente du serveur", "Il server non risponde ora", "サーバーの応答がありません", "服务器暂无最新应答", "O servidor não responde agora"}},
    {"block_old_data", {"The data of this battery is old", "Los datos de esta batería son antiguos", "Die Daten dieser Batterie sind alt", "Les données de cette batterie sont anciennes", "I dati di questa batteria sono vecchi", "この電池のデータは古いです", "此电池数据过旧", "Os dados desta bateria são antigos"}},
    {"block_example", {"This is an example, not a battery", "Es un ejemplo, no una batería", "Ein Beispiel, keine Batterie", "Un exemple, pas une batterie", "È un esempio, non una batteria", "これは見本データです", "这是示例数据", "É um exemplo, não uma bateria"}},
    {"block_no_pin", {"Open with the PIN first", "Abre antes con el PIN", "Zuerst mit PIN öffnen", "Ouvrez d'abord avec le code", "Apri prima con il PIN", "先に PIN で解除してください", "请先输入 PIN", "Abra primeiro com o PIN"}},
    {"order_not_sent", {"Sending orders is not written yet", "El envío de órdenes aún no está escrito", "Befehle senden ist noch nicht programmiert", "L'envoi d'ordres n'est pas encore écrit", "L'invio degli ordini non è ancora scritto", "命令の送信は未実装です", "指令发送尚未实现", "O envio de ordens ainda não está escrito"}},
    {"language", {"Language", "Idioma", "Sprache", "Langue", "Lingua", "言語", "语言", "Idioma"}},
    {"version", {"Version", "Versión", "Version", "Version", "Versione", "バージョン", "版本", "Versão"}},
    {"address", {"Network address", "Dirección de red", "Netzwerkadresse", "Adresse réseau", "Indirizzo di rete", "ネットワークアドレス", "网络地址", "Endereço de rede"}},
    {"no_address", {"No network", "Sin red", "Kein Netzwerk", "Pas de réseau", "Nessuna rete", "ネットワークなし", "无网络", "Sem rede"}},
    {"pin_none", {"No PIN is set: operations stay closed", "No hay PIN: las operaciones quedan cerradas", "Keine PIN gesetzt: Aktionen bleiben gesperrt", "Aucun code : les opérations restent fermées", "Nessun PIN: le operazioni restano chiuse", "PIN 未設定：操作は無効のままです", "未设置 PIN：操作保持关闭", "Sem PIN: as operações ficam fechadas"}},
    {"lock_now", {"Lock", "Bloquear", "Sperren", "Verrouiller", "Blocca", "ロック", "锁定", "Bloquear"}},
    {"diag_title", {"Board check", "Comprobación de la placa", "Platinentest", "Contrôle de la carte", "Controllo della scheda", "基板の確認", "电路板检查", "Verificação da placa"}},
    {"diag_chip", {"Chip", "Chip", "Chip", "Puce", "Chip", "チップ", "芯片", "Chip"}},
    {"diag_memory", {"Memory", "Memoria", "Speicher", "Mémoire", "Memoria", "メモリ", "内存", "Memória"}},
    {"diag_screen", {"Screen", "Pantalla", "Bildschirm", "Écran", "Schermo", "画面", "屏幕", "Ecrã"}},
    {"diag_touch", {"Touch", "Táctil", "Touch", "Tactile", "Tocco", "タッチ", "触摸", "Toque"}},
    {"diag_i2c", {"I2C bus", "Bus I2C", "I2C-Bus", "Bus I2C", "Bus I2C", "I2C バス", "I2C 总线", "Barramento I2C"}},
    {"diag_ethernet", {"Cable network", "Red por cable", "Kabelnetz", "Réseau filaire", "Rete via cavo", "有線ネットワーク", "有线网络", "Rede por cabo"}},
    {"diag_wifi", {"Wi-Fi radio", "Radio Wi-Fi", "WLAN-Funk", "Radio Wi-Fi", "Radio Wi-Fi", "Wi-Fi 無線", "Wi-Fi 无线", "Rádio Wi-Fi"}},
    {"diag_uptime", {"Time on", "Tiempo encendida", "Laufzeit", "Allumée depuis", "Accesa da", "稼働時間", "运行时间", "Tempo ligada"}},
    {"diag_hint", {"Touch each of the four corners. Red, green and blue must show in that order.", "Toca las cuatro esquinas. Deben verse rojo, verde y azul en ese orden.", "Alle vier Ecken berühren. Rot, Grün und Blau müssen in dieser Folge erscheinen.", "Touchez les quatre coins. Rouge, vert et bleu doivent apparaître dans cet ordre.", "Tocca i quattro angoli. Devono vedersi rosso, verde e blu in questo ordine.", "四隅をタッチしてください。赤、緑、青の順に表示されます。", "请触摸四个角。颜色应依次为红、绿、蓝。", "Toque nos quatro cantos. Deve ver vermelho, verde e azul por esta ordem."}},
    {"diag_open", {"Open the solar screen", "Abrir la pantalla solar", "Solarbildschirm öffnen", "Ouvrir l'écran solaire", "Apri lo schermo solare", "太陽光画面を開く", "打开太阳能界面", "Abrir o ecrã solar"}},
    {"diag_button", {"Board check", "Comprobar placa", "Platinentest", "Contrôle carte", "Controllo scheda", "基板の確認", "电路板检查", "Verificar placa"}},
};
// clang-format on

constexpr std::size_t kRowCount = sizeof(kRows) / sizeof(kRows[0]);

// The language the screen draws in: the asked one when it is one of the eight, else English.
inline std::string_view effective_language(std::string_view asked) {
  for (const std::string_view known : kLanguages)
    if (known == asked) return known;
  return kLanguages[0];
}

inline std::string_view word(std::string_view key, std::string_view language) {
  std::size_t column = 0;
  for (std::size_t i = 0; i < kLanguages.size(); ++i)
    if (kLanguages[i] == language) column = i;
  for (const Row& row : kRows)
    if (row.key == key) return row.text[column];
  return key;
}

inline bool known_key(std::string_view key) {
  for (const Row& row : kRows)
    if (row.key == key) return true;
  return false;
}

}  // namespace armor::solarhmi::text
