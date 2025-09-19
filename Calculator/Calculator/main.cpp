#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <utility>
#include <stack>
#include <cassert>

std::optional<std::string> exportNextElement(const std::string& stringExpression, const std::string& prevElement, int& i, std::string& errorMessage)
{
	char c = stringExpression[i];
	std::string ret = "";

	// Skip spaces, these are ignored
	while (c == ' ')
		c = stringExpression[++i];

	// These operators can exist on their own, and can be returned
	if (c == '+' || c == '*' || c == '/' || c == '(' || c == ')')
	{
		++i; 
		return std::string{ c };
	}

	// If the next character is a '-', we need to check whether it denotes a negative number, or a minus operator
	else if (c == '-' && (prevElement == "" || prevElement == "("))
	{
		ret += c;
		c = stringExpression[++i]; // Part of a negative number, now let's check for a number
	}
	else if (c == '-')
	{
		++i;
		return std::string{ c };
	}

	// Extract every digit of a number
	if (std::isdigit(c))
	{
		while (std::isdigit(c))
		{
			ret += c;
			c = stringExpression[++i];
		}
		return ret;
	}

	// Next valid element could not be found
	if (i < stringExpression.size())
	{
		errorMessage = "Garbage value found";
	}
	return std::nullopt;
}

bool createExpression(const std::string& stringExpression, std::vector<std::string>& expression, std::string& errorMessage)
{
	int i = 0;
	std::string prevElement = "";

	// This will determine how important operators are. Each bracket will increase the importance by one.
	int levelOfImportance = 0;

	bool numberExpected = true; // Flipflop (expression needs a number, then an operator, etc.)


	while (i < stringExpression.size())
	{
		std::optional<std::string> nextElement = exportNextElement(stringExpression, prevElement, i ,errorMessage);
		if (!nextElement.has_value())
		{
			// Expression ends in a number, with an apropriate size, generation successful
			if (!numberExpected && expression.size() % 2 == 1)
				return true;

			// Expression ends in an operator --> not good
			if(!errorMessage.size())
				errorMessage = "Expression ends in an operator.";
			return false;
		}

		std::string el = nextElement.value();

		// Alter level of importance
		if (el == "(")
			++levelOfImportance;
		else if (el == ")")
		{
			if (!levelOfImportance)
			{
				errorMessage = "Too many closing parentheses";
				return false;
			}

			--levelOfImportance;
		}

		// Denote importance of operator, depending on level of importnace
		if ((el == "+" || el == "-" || el == "*" || el == "/") && levelOfImportance)
		{
			std::string op = el;
			for (int j = 0; j < levelOfImportance; ++j)
				el += op;
		}

		if (el != "(" && el != ")")
		{
			// If we expect a number and get an operator, or expect an operator and get a number, fail the expression generation
			if (numberExpected && !std::isdigit(el[el.size() - 1]))
			{
				errorMessage = "Expected a number, but got an operator";
				return false;
			}
			else if (!numberExpected && std::isdigit(el[el.size() - 1]))
			{
				errorMessage = "Expected an operator, but got a number";
				return false;
			}

			numberExpected = !numberExpected; // Flip the flipflop

			expression.push_back(el); // Don't store parentheses, these are replaced by the level of importance
		}
		prevElement = el;
	}

	if (levelOfImportance)
	{
		errorMessage = "Parentheses not closed properly";
		return false;
	}

	return true;
}

/*
*	Operator importance is as follows:
*	- Equal number of digits: [+,-] < [*,/]
*	- Unequal number of operators [++,--] > [*,/] (note, the first operators are inside parentheses
*/

// This function will determine, if the left or right operator takes more importance
bool operator>=(std::string firstOperator, std::string secondOperator)
{
	// If the second operator is not valid, we can be sure that it is less than or equal to the first operator
	if (secondOperator == "") return true;
	if (firstOperator == "" && secondOperator != "") return false;

	// If the first operator has more characters than the second operator, it is of higher importance 
	if (firstOperator.size() > secondOperator.size()) return true;
	if (firstOperator.size() < secondOperator.size()) return false;

	// Otherwise standard order of operations
	if ((firstOperator[0] == '+' || firstOperator[0] == '-') && (secondOperator[0] == '*' || secondOperator[0] == '/')) return false;

	// Any other case is true
	return true;
}

int executeOperation(const std::string& firstNumber, const std::string& op, const std::string& secondNumber)
{
	int first = std::stoi(firstNumber);
	int second = std::stoi(secondNumber);
	char o = op[0];

	switch (o)
	{
	case '+':
		return first + second;
	case '-':
		return first - second;
	case '*':
		return first * second;
	case '/':
		return first / second;
	default:
		assert(false); // TODO: DO PROPER
		break;
	}
}

std::optional<int> calculate(const std::string& stringExpression, std::string& errorMessage)
{
	std::vector<std::string> expression;
	if (!createExpression(stringExpression, expression, errorMessage))
		return std::nullopt;

	// Not enough elements for a proper operation
	if (expression.size() < 3) return std::nullopt;

	std::stack<std::pair<std::string, std::string>> opStack;
	int sum = 0;
	int i = 0;

	std::string leftNumber = expression[i++];
	std::string firstOperator = expression[i++];
	std::string rightNumber = expression[i++];
	std::string secondOperator = i < expression.size() ? expression[i++] : "";

	bool expressionEvaluated = false;

	while (!expressionEvaluated)
	{
		// Check which operation to execute
		if (firstOperator >= secondOperator)
		{
			// No dividing by or with 0
			if (firstOperator[0] == '/' && (std::stoi(leftNumber) == 0 || std::stoi(rightNumber) == 0))
			{
				errorMessage = "Dividing by or with 0";
				return std::nullopt;
			}

			sum = executeOperation(leftNumber, firstOperator, rightNumber);

			// First check if the stack has an operation that takes presedence
			if (opStack.size() && opStack.top().second >= secondOperator)
			{
				leftNumber = opStack.top().first;
				firstOperator = opStack.top().second;
				rightNumber = std::to_string(sum);

				opStack.pop();
			}
			else
			{
				leftNumber = std::to_string(sum);
				firstOperator = secondOperator;
				rightNumber = i < expression.size() ? expression[i++] : "";
				secondOperator = i < expression.size() ? expression[i++] : "";
			}
		}

		// The second operator needs to be evaluated before the first one, so store the left hand number and it's operator on the stack
		else
		{
			opStack.push({ leftNumber, firstOperator });

			leftNumber = rightNumber;
			firstOperator = secondOperator;

			rightNumber = i < expression.size() ? expression[i++] : "";
			secondOperator = i < expression.size() ? expression[i++] : "";
		}

		expressionEvaluated = !rightNumber.size();
	}

	return sum;
}

int main()
{
	std::vector<std::pair<std::string, int>> testCases =
	{
		// Successful ones -- proper formatting
		{"1+2", 3},
		{"1-2", -1},
		{"2*2", 4},
		{"4/2", 2},
		{"2+3*5", 17},
		{"2*3-5", 1},
		{"10+2*3-5", 11},
		{"1+2-3*4/6+7-8*9/10", 1},
		{"1+(2-3)", 0},
		{"((1+2)-3)", 0},
		{"(-1-2)-(3-10)", 4},
		{"(1+2)-(3*9)*88", -2373},
		{"((1+2)-31)*4/((6+7)-(8*9))/10", 0},
		{"((125*34-789)/56+(882-945*2)*31-162)/(333-587*6)+147-274*89/111+(199-237)*421/93-(310-629/77)+552-131", -116},

		// Successful ones -- odd formatting
		{"1   +2", 3},
		{"1  -2", -1},
		{"       2*   2", 4},
		{"4/2", 2},
		{"2 + 3  *  5     ", 17},
		{"2   *3- 5", 1},
		{"10+2*3-5", 11},
		{"1+2 -3*4/6+7-8*9/10", 1},
		{"     1   +   (        2-3)", 0},
		{"((1   +2)-3)", 0},
		{"(    -1-2)-    (3-10)", 4},
		{"                                     (1+2    )-(3*9)*88  ", -2373},
		{"(    (   1+2)-31)*   4/((6+7)-   (8*9  )     )/10", 0},

		// Successful ones, but I put the wrong result
		{"1+1", 0},
		{"85*8581", 0},
		{"1+2+3+4+5+6+7+8+9-7-4-4-45+54+45-45+54+576-465-46+64-64-4+43+64-46+43", 0},

		// Unsuccessful ones -- nan
		{"1/0", 0},
		{"0/1", 0},
		{"(1-1)/0", 0},
		{"0/(1-1)", 0},

		// Unsuccessful ones -- unacceptable operations
		{"1 1", 0},
		{"1++1", 0},
		{"1*/1", 0},
		{"*5+5", 0},
		{"2*3-5*a", 0},
		{"I like kebab", 0}

	};

	for (std::pair<std::string, int>& tc : testCases)
	{
		std::string errorMessage = "";
		std::optional<int> res = calculate(tc.first, errorMessage);
		
		if (!res.has_value())
			std::cout << "Calculation unsuccessful, with error \"" << errorMessage << "\" [" << tc.first << "]" << std::endl;
		else if (res.value() != tc.second)
			std::cout << "Calculation unsuccessful, since the result \"" << res.value() << "\" does not equal the expected " << tc.second << " [" << tc.first << "]." << std::endl;
		else
			std::cout << "Calculation successful succesfully, \"" << res.value() << "\" == \"" << tc.second << "\". [" << tc.first << "]" << std::endl;
	}

	std::cout << std::endl << std::endl;

	std::string userInput = "";

	while(true) {
		std::cout << "Now try your own equation (or 'q' to quit): ";
		userInput = "";
		std::getline(std::cin, userInput);
		if ((userInput == "q"))
		{
			std::cout << "\n\n\n\n\n\n\nGoodbye! (and hire me pls)\n\n\n\n\n\n\n";
			return 0;
		}

		std::string errorMessage = "";
		std::optional<int> res = calculate(userInput, errorMessage);

		if (!res.has_value())
		{
			std::cout << "\nUnsuccessful, with error message \"" << errorMessage << "\" try again." << std::endl;
			continue;
		}

		std::cout << "Result: \"" << res.value() << "\"." << std::endl;

	}



	return 0;
}