#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <utility>
#include <stack>
#include <cassert>

std::optional<std::string> exportNextElement(const std::string& stringExpression, const std::string& prevElement, int& i)
{
	char c = stringExpression[i];
	std::string ret = "";

	// Skip spaces, these are ignored
	while (c == ' ')
		++i;

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
	else if (std::isdigit(c))
	{
		while (std::isdigit(c))
		{
			ret += c;
			c = stringExpression[++i];
		}
		return ret;
	}

	// Next valid element could not be found
	return std::nullopt;
}

bool createExpression(const std::string& stringExpression, std::vector<std::string>& expression)
{
	int i = 0;
	std::string prevElement = "";

	// This will determine how important operators are. Each bracket will increase the importance by one.
	int levelOfImportance = 0;


	while (i < stringExpression.size())
	{
		std::optional<std::string> nextElement = exportNextElement(stringExpression, prevElement, i);
		if (!nextElement.has_value())
			return false;

		std::string el = nextElement.value();

		// Alter level of importance
		if (el == "(")
			++levelOfImportance;
		else if (el == ")")
		{
			if (!levelOfImportance)
				return false; // ERROR: Too many closing parentheses

			--levelOfImportance;
		}

		// Denote importance of operator, depending on level of importnace
		if ((el == "+" || el == "-" || el == "*" || el == "/") && levelOfImportance)
		{
			std::string op = el;
			for (int j = 0; j < levelOfImportance; ++j)
				el += op;
		}

		if(el != "(" && el != ")")
			expression.push_back(el); // Don't store parentheses, these are replaced by the level of importance
		prevElement = el;
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

std::optional<int> calculate(const std::vector<std::string>& expression)
{
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
			sum = executeOperation(leftNumber, firstOperator, rightNumber);

			leftNumber = std::to_string(sum);
			firstOperator = secondOperator;
			rightNumber = i < expression.size() ? expression[i++] : "";
			secondOperator = i < expression.size() ? expression[i++] : "";
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
		{"1+2", 3},
		{"1+(2-3)", 0},
	};

	for (std::pair<std::string, int>& tc : testCases)
	{
		std::vector<std::string> expression;
		if (!createExpression(tc.first, expression))
			assert(false);

		std::optional<int> res = calculate(expression);
	}

	return 0;
}