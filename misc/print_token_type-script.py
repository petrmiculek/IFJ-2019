"""
Skript na tvorbu if-else if bloku pro tisk typu tokenu

1. prekopirovat do token_type ENUM ze scanner.h
2. ctrl + h, nahradit TOKEN_ za '       # otevreni stringu
3.           nahradit , za ',           # uzavreni stringu
4. prepsat zavorky na hranate

Zkopirovat vystup do programu do zdroje ( smazat prvni else )

tokeny aktualni k datu 16.11.2019
"""
token_type = ['INT', 'FLOAT', 'UP', 'DOWN', 'LEFT', 'RIGHT', 'LIT', 'NEG', 'IS_EQUAL', 'N_EQUAL', 'MORE', 'MORE_E',
              'LESS', 'LESS_E', 'MULTI', 'DIVISION', 'FLOR_DIV', 'MINUS', 'PLUS', 'NONE', 'PASS', 'RETURN', 'COMMA',
              'COLON', 'SPACE', 'DEF', 'IF', 'ELSE', 'WHILE', 'ASSIGN', 'INDENT', 'DEDENT', 'END', 'EOL', 'EOF',
              'IDENTIFIER', ]
for i in token_type:
    print("""else if (token.type == TOKEN_%s)
        {
            printf("%s\\n");
        }""" % (i, i))
