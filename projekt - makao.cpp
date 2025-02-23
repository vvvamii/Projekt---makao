#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // random shuffle

using namespace std;

// 0 - symbole; 1 - francuski; 2 - niemiecki / preferowane 0; w przypadku problemów z wyświetlaniem ustawić 1
const int SUIT_TYPE = 0;
const string values[13] = { "A","2","3","4","5","6","7","8","9","10","J","Q","K" };
const string colors0[4] = { "♦","♠","♣","♥" };
const string colors1[4] = { "karo","pik","trefl","kier" };
const string colors2[4] = { "dzwonek","wino","żołądź","czerwień" };

struct card {
    string value;
    string color;
    int type;
    bool legal;
    bool counter;
};

struct player {
    string name;
    vector<card> hand;
    int waitingTurns;
    bool bot;
    bool makao;
};

struct gameEvent {
    int type;
    int howMany;
    int waitingTurns;
    int turnsJack;
    int turnsAce;
    string value;
    string color;
    card previousCard;
};

// Zainicjuj talię + przetasuj karty
void initDeck(vector<card>& cards) {
    card templatecard;
    for (int i = 0; i < 52; i++) {

        templatecard.value = values[i % 13];

        /**
        switch (SUIT_TYPE)
        {
        case 0:
            templatecard.color = colors0[i / 13];
            break;
        case 1:
            templatecard.color = colors1[i / 13];
            break;
        case 2:
            templatecard.color = colors2[i / 13];
            break;
        }
        */

        templatecard.color = colors1[i / 13];

        cards.push_back(templatecard);
    }

    for (int i = 0; i < 52; ++i) {
        if (cards[i].value == "A") cards[i].type = 1;   // zmiana koloru (dotycząca tylko kolejnego gracza)
        else if (cards[i].value == "2") cards[i].type = 2;  // kolejny gracz dobiera 2 karty
        else if (cards[i].value == "3") cards[i].type = 3;  // kolejny gracz dobiera 3 karty
        else if (cards[i].value == "4") cards[i].type = 4;  // kolejny gracz czeka kolejkę

        //[...] dobiera pięć kart
        else if (cards[i].value == "K" && cards[i].color == "pik") cards[i].type = 5;   // [poprzedni gracz]
        else if (cards[i].value == "K" && cards[i].color == "kier") cards[i].type = 6;  // [następny gracz]
        else if (cards[i].value == "K" && (cards[i].color == "trefl" || cards[i].color == "karo")) cards[i].type = 7;   // blokada

        else if (cards[i].value == "J") cards[i].type = 8;  // żądanie kart według ich wartości przez całą kolejkę (do zagrywającego włącznie)
        else if (cards[i].value == "Q") cards[i].type = 9;  // dama na wszystko, wszystko na damę (oprócz J)

        else cards[i].type = 0; // inne karty

        cards[i].legal = false;
        cards[i].counter = false;
    }

    random_shuffle(cards.begin(), cards.end());
}

string chosenColorDisplayMode(card card) {
    string cardColor;

    switch (SUIT_TYPE)
    {
    case 0:
        if (card.color == "karo") cardColor = colors0[0];
        else if (card.color == "pik") cardColor = colors0[1];
        else if (card.color == "trefl") cardColor = colors0[2];
        else cardColor = colors0[3];
        break;
    case 1:
        cardColor = card.color;
        break;
    case 2:
        if (card.color == "karo") cardColor = colors2[0];
        else if (card.color == "pik") cardColor = colors2[1];
        else if (card.color == "trefl") cardColor = colors2[2];
        else cardColor = colors2[3];
        break;
    }

    return cardColor;
}

// Wyświetl (wszystkie) karty z vectora
void viewCards(vector<card>& cards, bool allCards, int singleCardPosition = 0) {
    string cardString;
    string cardStringColor;
    if (allCards == true) {
        for (int i = 0; i < cards.size(); ++i) {
            cardStringColor = chosenColorDisplayMode(cards[i]);

            if (cards[i].legal == true) cardString = "*";
            if (cards[i].counter == true) cardString = "C";

            cardString += "[" + cards[i].value + "|" + cardStringColor + "] - " + to_string(i + 1) + "\t";
            cout << cardString << " ";
            cardString.clear();
            cardStringColor.clear();
        }
    }
    else {
        cardStringColor = chosenColorDisplayMode(cards[singleCardPosition]);
        cardString = "[" + cards[singleCardPosition].value + "|" + cardStringColor + "]";
        cout << cardString << "\t";
        cardString.clear();
        cardStringColor.clear();
    }
}

// Przekłada kartę z jednego vectora do drugiego
void takeCardFromOneVectorToAnother(vector<card>& cardsFrom, vector<card>& cardsTo, int position) {
    cardsTo.push_back(cardsFrom[position]);
    cardsFrom.erase(cardsFrom.begin() + position);
}

void checkIfLegal(card& handCard, card& cardsPlayedTop, gameEvent& gameEvent) {

    if (gameEvent.turnsJack > 0) {
        if (handCard.type == 8) {
            handCard.counter = true;
            handCard.legal = true;
        }
        else if (handCard.value == gameEvent.value) handCard.legal = true;
        else handCard.legal = false;
    }

    else if (gameEvent.turnsJack < 1) {
        if (gameEvent.type == 0) {
            if (handCard.color == cardsPlayedTop.color || handCard.value == cardsPlayedTop.value) handCard.legal = true;
            else handCard.legal = false;
        }

        else if (gameEvent.type == 1 && gameEvent.turnsAce > 0) {
            if (handCard.color == gameEvent.color && handCard.value != "A") handCard.legal = true;
            else if (handCard.value == "A") {
                handCard.legal = true;
                handCard.counter = true;
            }
            else handCard.legal = false;
        }

        else if (gameEvent.type == 1 && gameEvent.turnsAce < 1) {
            if (handCard.color == gameEvent.previousCard.color || handCard.value == gameEvent.previousCard.value) handCard.legal = true;
            else handCard.legal = false;
        }

        else if (gameEvent.type == 2) {
            if (handCard.color == cardsPlayedTop.color) handCard.legal = true;
            else if (handCard.type == 2) {
                handCard.legal = true;
                handCard.counter = true;
            }
            else handCard.legal = false;
        }

        else if (gameEvent.type == 3) {
            if (handCard.color == cardsPlayedTop.color) handCard.legal = true;
            else if (handCard.type == 3) {
                handCard.legal = true;
                handCard.counter = true;

            }
            else handCard.legal = false;
        }

        else if (gameEvent.type == 4) {
            if (handCard.type == 4) {
                handCard.legal = true;
                handCard.counter = true;
            }
            else handCard.legal = false;
        }

        else if (gameEvent.type == 5 || gameEvent.type == 6) {
            if (handCard.type == 5 || handCard.type == 6 || handCard.type == 7) {
                handCard.legal = true;
                handCard.counter = true;
            }
            else if (handCard.color == cardsPlayedTop.color) handCard.legal = true;
            else handCard.legal = false;
        }

        else if (gameEvent.type == 8) {
            if (handCard.value == gameEvent.previousCard.value) handCard.legal = true;
            else handCard.legal = false;
        }

        else handCard.legal = false;

        if (handCard.type == 9) {
            if (gameEvent.type != 1 && gameEvent.type != 4 && gameEvent.type != 8) {
                handCard.legal = true;
            }
            else if (gameEvent.type == 1) {
                if (handCard.color == gameEvent.color) handCard.legal = true;
                else handCard.legal = false;
            }
            else handCard.legal = false;
        }

        if (cardsPlayedTop.type == 9) handCard.legal = true;
    }
}

void checkIfNotEnoughCards(vector<card>& deck, vector<card>& cardsPlayed) {
    if (deck.size() == 0) {
        for (int i = 0; i < (cardsPlayed.size() - 1); ++i) {
            if (deck.size() != 0) takeCardFromOneVectorToAnother(cardsPlayed, deck, 0);
        }
    }
    random_shuffle(deck.begin(), deck.end());
}

void takeCards(vector<card>& hand, vector<card>& deck, vector<card>& cardsPlayed, gameEvent& gameEvent) {
    for (int i = 0; i < gameEvent.howMany; ++i) {
        checkIfNotEnoughCards(deck, cardsPlayed);
        takeCardFromOneVectorToAnother(deck, hand, 0);
    }

    gameEvent.howMany = 0;
    gameEvent.type = cardsPlayed[cardsPlayed.size() - 1].type;
}

void eventKing(vector<player>& players, vector<card>& deck, vector<card>& cardsPlayed, gameEvent& gameEvent, int currentPlayerIndex) {
    if (gameEvent.type == 5) {
        int previousPlayerIndex = 0;
        if (currentPlayerIndex > 1) previousPlayerIndex = currentPlayerIndex - 2;
        else if (currentPlayerIndex == 1) previousPlayerIndex = players.size() - 1;
        else if (currentPlayerIndex == 0) previousPlayerIndex = players.size() - 2;
        takeCards(players[previousPlayerIndex].hand, deck, cardsPlayed, gameEvent);
    }
    else if (gameEvent.type == 6) {
        int nextPlayerIndex = currentPlayerIndex;
        takeCards(players[nextPlayerIndex].hand, deck, cardsPlayed, gameEvent);
    }
}

void applyEvents(vector<player>& players, vector<card>& deck, vector<card>& cardsPlayed, gameEvent& gameEvent, int currentPlayerIndex) {
    switch (gameEvent.type)
    {
    case 0:
        break;

    case 1:
        gameEvent.turnsAce--;
        break;

    case 2:
        takeCards(players[currentPlayerIndex].hand, deck, cardsPlayed, gameEvent);
        break;

    case 3:
        takeCards(players[currentPlayerIndex].hand, deck, cardsPlayed, gameEvent);
        break;

    case 4:
        players[currentPlayerIndex].waitingTurns = gameEvent.waitingTurns;
        gameEvent.waitingTurns = 0;
        gameEvent.type = cardsPlayed[cardsPlayed.size() - 1].type;
        break;

    case 5:
        eventKing(players, deck, cardsPlayed, gameEvent, currentPlayerIndex);
        break;

    case 6:
        eventKing(players, deck, cardsPlayed, gameEvent, currentPlayerIndex);
        break;

    case 7:
        break;

    case 8:
        gameEvent.turnsJack--;
        break;

    default:
        break;
    }

}

void addEventAce(vector<card>& hand, vector<card>& cardsPlayed, gameEvent& gameEvent, bool bot) {

    if (gameEvent.type != 1) gameEvent.previousCard = cardsPlayed[cardsPlayed.size() - 1];

    string choose;
    bool end = false;

    if (bot == false) {
        cout << endl << "Wybierz kolor ( karo / pik / trefl / kier ) >";
        do {
            cin >> choose;
            if (choose == "karo" || choose == "pik" || choose == "trefl" || choose == "kier") {
                gameEvent.color = choose;
                end = true;
            }
            else cout << "Wybierz właściwy kolor ( karo / pik / trefl / kier ) >";
        } while (end == false);
    }
    else gameEvent.color = gameEvent.previousCard.color;

    gameEvent.type = 1;
    gameEvent.turnsAce = 1;
}

void addEventJack(vector<player>& players, vector<card>& cardsPlayed, gameEvent& gameEvent, int cardIndex, int currentPlayerIndex) {

    if (gameEvent.type != 8) gameEvent.previousCard = cardsPlayed[cardsPlayed.size() - 1];

    string choose;
    bool end = false;
    if (players[currentPlayerIndex].bot == false) {
        cout << endl << "Wybierz wartość ( 5 / 6 / 7 / 8 / 9 / 10 ) >";
        do {
            cin >> choose;
            if (choose == "5" || choose == "6" || choose == "7" || choose == "8" || choose == "9" || choose == "10") {
                gameEvent.value = choose;
                end = true;
            }
            else cout << "Wybierz właściwą wartość ( 5 / 6 / 7 / 8 / 9 / 10 ) >";
        } while (end == false);

        gameEvent.type = 8;
        gameEvent.turnsJack = players.size();

    }
    else gameEvent.value = gameEvent.previousCard.value;
}

void addEvents(vector<player>& players, vector<card> deck, vector<card> cardsPlayed, gameEvent& gameEvent, int cardIndex, int currentPlayerIndex) {
    switch (players[currentPlayerIndex].hand[cardIndex].type)
    {
    case 0:
        break;

    case 1:
        addEventAce(players[currentPlayerIndex].hand, cardsPlayed, gameEvent, players[currentPlayerIndex].bot);
        break;

    case 2:
        gameEvent.type = 2;
        gameEvent.howMany = 2;
        break;

    case 3:
        gameEvent.type = 3;
        gameEvent.howMany = 3;
        break;

    case 4:
        gameEvent.type = 4;
        gameEvent.waitingTurns = 1;
        break;

    case 5:
        gameEvent.type = 5;
        gameEvent.howMany = 5;
        break;

    case 6:
        gameEvent.type = 6;
        gameEvent.howMany = 5;
        break;

    case 7:
        gameEvent.type = 0;
        gameEvent.howMany = 0;
        break;

    case 8:
        addEventJack(players, cardsPlayed, gameEvent, cardIndex, currentPlayerIndex);
        break;

    default:
        break;
    }
}

void moveCardKing(vector<card>& hand, vector<card>& cardsPlayed, gameEvent& gameEvent, int cardIndex) {
    if (gameEvent.type == 5) {
        if (hand[cardIndex].type == 6) {
            gameEvent.howMany += 5;
            gameEvent.type = 6;
        }
        else if (hand[cardIndex].type == 7) {
            gameEvent.howMany = 0;
            gameEvent.type = 0;
        }
    }

    else if (gameEvent.type == 6) {
        if (hand[cardIndex].type == 5) {
            gameEvent.howMany += 5;
            gameEvent.type = 5;
        }
        else if (hand[cardIndex].type == 7) {
            gameEvent.howMany = 0;
            gameEvent.type = 0;
        }
    }

    takeCardFromOneVectorToAnother(hand, cardsPlayed, cardIndex);
}



void moveCard(vector<player>& players, vector<card>& deck, vector<card>& cardsPlayed, gameEvent& gameEvent, int cardIndex, int currentPlayerIndex) {
    if (players[currentPlayerIndex].hand[cardIndex].counter == true) {
        switch (gameEvent.type)
        {
        case 1:
            addEventAce(players[currentPlayerIndex].hand, cardsPlayed, gameEvent, players[currentPlayerIndex].bot);
            break;

        case 2:
            gameEvent.howMany += 2;
            takeCardFromOneVectorToAnother(players[currentPlayerIndex].hand, cardsPlayed, cardIndex);
            break;

        case 3:
            gameEvent.howMany += 3;
            takeCardFromOneVectorToAnother(players[currentPlayerIndex].hand, cardsPlayed, cardIndex);
            break;

        case 4:
            gameEvent.waitingTurns++;
            takeCardFromOneVectorToAnother(players[currentPlayerIndex].hand, cardsPlayed, cardIndex);
            break;

        case 5:

        case 6:
            moveCardKing(players[currentPlayerIndex].hand, cardsPlayed, gameEvent, cardIndex);
            break;

        case 7:
            break;

        case 8:
            addEventJack(players, cardsPlayed, gameEvent, cardIndex, currentPlayerIndex);
            break;

        default:
            break;
        }
    }

    else if (players[currentPlayerIndex].hand[cardIndex].counter == false && players[currentPlayerIndex].hand[cardIndex].legal == true) {
        applyEvents(players, deck, cardsPlayed, gameEvent, currentPlayerIndex);
        addEvents(players, deck, cardsPlayed, gameEvent, cardIndex, currentPlayerIndex);
        takeCardFromOneVectorToAnother(players[currentPlayerIndex].hand, cardsPlayed, cardIndex);
    }
}

void cheat(vector<player>& players) {
    for (auto elem : players) {
        cout << endl << "Karty gracza " << elem.name << ":" << endl << endl;
        viewCards(elem.hand, true);
        cout << endl << endl << "=======================================================" << endl;
    }
}

void play(vector<player>& players, vector<card>& deck, vector<card>& cardsPlayed, gameEvent& gameEvent, int currentPlayerIndex, int index = 0, bool addedCard = false) {
    int cardIndex = 0;
    if (players[currentPlayerIndex].bot == false) {
        if (addedCard == false) {
            if (players[currentPlayerIndex].hand.size() == 1 && players[currentPlayerIndex].makao == false) {
                cout << "Nie wybrałeś opcji makao pomimo posiadania jednej karty - dobierasz 5" << endl;
                for (int j = 0; j < 5; ++j) {
                    checkIfNotEnoughCards(deck, cardsPlayed);
                    takeCardFromOneVectorToAnother(deck, players[currentPlayerIndex].hand, 0);
                }
            }
            else {
                do {
                    cout << "Podaj numer karty jaki chcesz zagrać (np. pierwsza karta - wpisz 1)" << endl << ">";
                    cin >> cardIndex;
                    cardIndex = cardIndex - 1;

                    if (players[currentPlayerIndex].hand[cardIndex].legal == false) cout << "Nie jest to dozwolone zagranie" << endl << ">";

                } while (players[currentPlayerIndex].hand[cardIndex].legal == false);

                moveCard(players, deck, cardsPlayed, gameEvent, cardIndex, currentPlayerIndex);
            }
        }
        else moveCard(players, deck, cardsPlayed, gameEvent, index, currentPlayerIndex);
    }

    else {
        cardIndex = 111;
        for (int j = 0; j < players[currentPlayerIndex].hand.size(); ++j) {
            checkIfLegal(players[currentPlayerIndex].hand[j], cardsPlayed[cardsPlayed.size() - 1], gameEvent);
            if (players[currentPlayerIndex].hand[j].legal == true) cardIndex = j;
        }
        if (cardIndex == 111) {
            checkIfNotEnoughCards(deck, cardsPlayed);
            takeCardFromOneVectorToAnother(deck, players[currentPlayerIndex].hand, 0);
            checkIfLegal(players[currentPlayerIndex].hand[players[currentPlayerIndex].hand.size() - 1], cardsPlayed[cardsPlayed.size() - 1], gameEvent);
            if (players[currentPlayerIndex].hand[players[currentPlayerIndex].hand.size() - 1].legal == true) moveCard(players, deck, cardsPlayed, gameEvent, (players[currentPlayerIndex].hand.size() - 1), currentPlayerIndex);
            else applyEvents(players, deck, cardsPlayed, gameEvent, currentPlayerIndex);
        }
        else {
            moveCard(players, deck, cardsPlayed, gameEvent, cardIndex, currentPlayerIndex);
        }
    }

}

void displayCurrentEvents(gameEvent gameEvent) {
    cout << endl << endl;
    if (gameEvent.type == 1) cout << "AS - zagrywasz do koloru - " << gameEvent.color << endl;
    else if (gameEvent.howMany > 0 && gameEvent.type == 2 || gameEvent.type == 3 || gameEvent.type == 6) cout << "Dobierasz w tej turze " << gameEvent.howMany << " kart/y" << endl;
    else if (gameEvent.howMany > 0 && gameEvent.type == 5) cout << "Poprzedni gracz dobiera " << gameEvent.howMany << " kart" << endl;
    else if (gameEvent.type == 4) cout << "Po tej turze czekasz " << gameEvent.waitingTurns << "tur" << endl;
    else if (gameEvent.type == 8) cout << "WALET - zagrywasz do figury - " << gameEvent.value << " | Pozostało " << gameEvent.turnsJack << " tur" << endl;
}

// Mechanizm odpowiadający za tury + komendy
void turn(vector<player>& players, vector<card>& deck, vector<card>& cardsPlayed, bool& isGame) {
    gameEvent gameEvent;
    gameEvent.type = 0;
    gameEvent.howMany - 0;
    gameEvent.turnsJack = 0;
    gameEvent.turnsAce = 0;
    gameEvent.waitingTurns = 0;
    string command;
    bool turnEnd = false;
    bool noLegalCardInHand = true;
    int checkHowManyPlayersEnded = 0;

    while (isGame == true) {
        for (int i = 0; i < players.size(); ++i) {

            if (gameEvent.turnsJack < 1 && gameEvent.type == 8) {
                gameEvent.type = 0;
                gameEvent.turnsJack = 0;

                for (int j = 0; j < cardsPlayed.size(); ++j) {
                    if (cardsPlayed[j].color == gameEvent.previousCard.color && cardsPlayed[j].value == gameEvent.previousCard.value) {
                        takeCardFromOneVectorToAnother(cardsPlayed, cardsPlayed, j);
                    }
                }
            }

            if (gameEvent.turnsAce < 1 && gameEvent.type == 1) {
                gameEvent.type = 0;
                gameEvent.turnsAce = 0;

                for (int j = 0; j < cardsPlayed.size(); ++j) {
                    if (cardsPlayed[j].color == gameEvent.previousCard.color && cardsPlayed[j].value == gameEvent.previousCard.value) {
                        takeCardFromOneVectorToAnother(cardsPlayed, cardsPlayed, j);
                    }
                }
            }

            if (gameEvent.waitingTurns < 1 && gameEvent.type == 4) {
                gameEvent.type = 0;
            }

            for (auto& elem : players[i].hand) {
                elem.legal = false;
                elem.counter = false;
            }
            noLegalCardInHand = true;

            if (players[i].bot == false && players[i].waitingTurns == 0) {
                system("cls"); // czyszczenie konsoli
                for (int j = 0; j < players.size(); ++j) {
                    if (i == j) cout << ">>> " << players[i].name << " = " << players[i].hand.size() << " <<<\t\t";
                    else cout << players[j].name << " = " << players[j].hand.size() << "\t\t";
                }

                for (auto& elem : players[i].hand) {
                    checkIfLegal(elem, cardsPlayed[cardsPlayed.size() - 1], gameEvent);
                    // cout << elem.legal << " ";
                    if (elem.legal == true) noLegalCardInHand = false;
                }

                displayCurrentEvents(gameEvent);

                //  cout << endl << "DECK.SIZE(): " << deck.size() << endl;
                //  cout << endl << "GAMEEVENT.TYPE = " << gameEvent.type << endl;

                cout << endl << endl << "Tura gracza: " << players[i].name << endl << endl << "Twoje karty:" << endl;
                viewCards(players[i].hand, true);
                cout << endl << endl;

                if (gameEvent.type != 1 && gameEvent.type != 8) {
                    cout << "Karta, pod którą zagrywasz:\t\t";
                    viewCards(cardsPlayed, false, (cardsPlayed.size() - 1));
                }

                turnEnd = false;
                cout << endl;

                do {
                    cout << endl << endl << "VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV" << endl << endl;
                    cout << "quit / play / makao / cheat / pass" << endl;
                    cout << "Jaki jest twój ruch? >";
                    cin >> command;

                    if (command == "cheat") {
                        cheat(players);
                    }

                    else if (command == "quit") {
                        exit(0);
                    }

                    else if (command == "play") {

                        if (noLegalCardInHand == true) {
                            cout << endl << "Brak legalnych ruchów - dobierasz jedną kartę ze stosu" << endl;
                            takeCardFromOneVectorToAnother(deck, players[i].hand, 0);
                            checkIfLegal(players[i].hand[players[i].hand.size() - 1], cardsPlayed[cardsPlayed.size() - 1], gameEvent);
                            viewCards(players[i].hand, true);
                            cout << endl;
                            system("pause");

                            if (players[i].hand[players[i].hand.size() - 1].legal == true) {
                                cout << endl << "Możesz rzucić pobraną kartę, Tak/Nie >";
                                cin >> command;
                                if (command == "t" || command == "T") {
                                    play(players, deck, cardsPlayed, gameEvent, i, (players[i].hand.size() - 1), true);
                                    turnEnd = true;
                                }
                                else if (command == "n" || command == "N") turnEnd = true;
                                else cout << endl << "Wprowadź poprawną komendę (t/n) >";
                            }
                            else {
                                cout << endl << "Pobrana karta nie jest legalna - koniec tury" << endl;
                                applyEvents(players, deck, cardsPlayed, gameEvent, i);
                                turnEnd = true;
                            }
                        }
                        else {
                            play(players, deck, cardsPlayed, gameEvent, i);
                        }

                        if (players[i].hand.size() != 1) {
                            players[i].makao = false;
                            turnEnd = true;
                        }
                    }

                    else if (command == "makao") {
                        if (players[i].hand.size() == 1) {
                            players[i].makao = true;
                            turnEnd = true;
                        }
                    }

                    else if (command == "pass") {
                        applyEvents(players, deck, cardsPlayed, gameEvent, i);
                        takeCardFromOneVectorToAnother(deck, players[i].hand, 0);
                        turnEnd = true;
                    }

                    else cout << endl << "Wprowadź poprawną komendę (play/makao/cheat/quit) >";

                } while (turnEnd == false);

            }

            else if (players[i].bot == false && players[i].waitingTurns != 0) {
                players[i].waitingTurns--;
            }

            else if (players[i].bot == true) {
                play(players, deck, cardsPlayed, gameEvent, i);
                viewCards(players[i].hand, true);
                // cout << endl << "Karty bota po jego turze: " << endl;
                // system("pause");
            }

            if (players[i].hand.size() == 0) {
                cout << "Gracz " << players[i].name << " pozbył się wszystkich swoich kart" << endl;
                players.erase(players.begin() + i);
                system("pause");
            }

            if (players.size() == 1) {
                isGame = false;
                cout << "Koniec gry - pozostał tylko gracz " << players[0].name << "!" << endl;
                system("pause");
            }
        }
    }

}

// Start rozgrywki (podanie liczby graczy)
void start(vector<player>& players, vector<card>& deck, vector<card>& playedCards) {
    int howManyPlayers = 0;
    int howManyBots = 0;
    cout << "MAKAO" << endl;

    do {
        cout << "Podaj liczbę graczy (2 - 4): " << endl;
        cin >> howManyPlayers;
        while (cin.fail()) {
            cin.clear();
            cin.ignore();
            cout << "Wpisz liczbę graczy (od 2 do 4)" << endl;
            cin >> howManyPlayers;
        }
    } while (howManyPlayers != 2 && howManyPlayers != 3 && howManyPlayers != 4);

    do {
        cout << "Podaj liczbę botów (0 - " << howManyPlayers - 1 << ")" << endl;
        cin >> howManyBots;
        while (cin.fail()) {
            cin.clear();
            cin.ignore();
            cout << "Wpisz liczbę botów (0 - " << howManyPlayers - 1 << ")" << endl;
            cin >> howManyBots;
        }
    } while (howManyBots >= howManyPlayers || howManyBots < 0);

    cout << endl << endl << "Gra rozpocznie się z " << howManyPlayers << " graczami, z czego " << howManyBots << " to bot(y)" << endl;

    players.resize(howManyPlayers);

    for (int i = 0; i < howManyPlayers; ++i) {
        if (howManyPlayers - howManyBots > i) {
            players[i].bot = false;
            cout << "Gracz " << i + 1 << ", jak się nazywasz ";
            cin >> players[i].name;
            cout << endl << endl;
        }
        else {
            players[i].bot = true;
            players[i].name = "GRACZ " + to_string(i + 1) + "(BOT)";

        }
        for (int j = 0; j < 5; ++j) {
            takeCardFromOneVectorToAnother(deck, players[i].hand, 0);
        }
        players[i].makao = false;
    }

    takeCardFromOneVectorToAnother(deck, playedCards, 0);
}

// Zainicjalizowanie gry (połącz wszystkie funkcje w całość)
void initGame() {
    bool isGame = true;
    vector<card> deck;
    vector<card> playedCards;
    vector<player> players;
    initDeck(deck);

    viewCards(deck, true);
    cout << endl << endl;

    start(players, deck, playedCards);
    turn(players, deck, playedCards, isGame);

}

int main()
{
    srand(time(0));
    initGame();
}