#include <algorithm>
#include <iostream>
#include <list>
#include <thread>
#include "tournament.hpp"

Tournament::Tournament(const Tournament& previous, const float& mutationIntensity, std::mt19937& generator) : contestants(previous.contestants) {
	std::sort(this->contestants.begin(), this->contestants.end(), [](auto& a, auto& b){
		// reversed comparison because we want descending order
		return std::get<1>(a) > std::get<1>(b); });
	this->contestants.reserve(this->contestants.size());
	this->contestants.resize(this->contestants.size() / 2);
	for (std::tuple<Bot, int>& contestant : this->contestants) {
		this->contestants.emplace_back(Bot(std::get<0>(contestant), mutationIntensity, generator), 0);
		std::get<1>(contestant) = 0;
	}
}

bool Tournament::addContestant(const Bot& newContestant) {
	if (std::find_if(this->contestants.begin(), this->contestants.end(), [&newContestant](auto a){ return std::get<0>(a) == newContestant; }) != this->contestants.end()) {
		return false;
	}
	this->contestants.emplace_back(newContestant, 0);
	return true;
}

void Tournament::evaluate(const bool loud) {
	std::list<std::thread> games;
	for (auto it = this->contestants.begin(); it != this->contestants.end(); ++it) {
		for (auto jt = this->contestants.begin(); jt != this->contestants.end(); ++jt) {
			if (it != jt) {
				if (games.size() >= std::thread::hardware_concurrency() - 1) {
					games.begin()->join();
					games.pop_front();
				}
				games.emplace_back(&Tournament::playGame, this, it, jt, true, loud);
				//this->playGame(it, jt, true, loud);
			} else if (loud) {
				std::cout << "-";
			}
		}
		if (loud) {
			std::cout << "\n";
			flush(std::cout);
		}
	}
	while (games.size() > 0) {
		games.begin()->join();
		games.pop_front();
	}
}

void Tournament::playGame(std::vector<std::tuple<Bot, int>>::iterator bot1, std::vector<std::tuple<Bot, int>>::iterator bot2, const bool rated, const bool loud) {
	std::vector<Board> board_history;
	std::vector<Board>::reverse_iterator current_situation;
	Board new_situation;
	board_history.emplace_back();
	current_situation = board_history.rbegin();
	Move current_move;
	unsigned long long int timesum_step;
	while (true) {
		timesum_step = Bot::timesum;
		for (int i = 0; i < 100 && (Bot::timesum - timesum_step) / CLOCKS_PER_SEC < 0.01; ++i) {
			std::tie(current_move, std::ignore) = std::get<0>(*bot1).getQuickMove(*current_situation, i);
		}
		new_situation = current_situation->applyMove(current_move, true);
		if (!new_situation.isValid()) {
			if (rated) {
				std::get<1>(*bot1) += 3;
			}
			if (loud) {
				std::cout << "<";
			}
			return;
		} else if (std::count(board_history.begin(), board_history.end(), new_situation) > 2) {
			if (rated) {
				std::get<1>(*bot1) += 1;
				std::get<1>(*bot2) += 1;
			}
			if (loud) {
				std::cout << ".";
			}
			return;
		}
		board_history.push_back(new_situation);
		current_situation = board_history.rbegin();
		timesum_step = Bot::timesum;
		for (int i = 0; i < 100 && (Bot::timesum - timesum_step) / CLOCKS_PER_SEC < 0.01; ++i) {
			std::tie(current_move, std::ignore) = std::get<0>(*bot2).getQuickMove(*current_situation, i);
		}
		new_situation = current_situation->applyMove(current_move, true);
		if (!new_situation.isValid()) {
			if (rated) {
				std::get<1>(*bot2) += 3;
			}
			if (loud) {
				std::cout << ">";
			}
			return;
		}if (std::count(board_history.begin(), board_history.end(), new_situation) > 2) {
			if (rated) {
				std::get<1>(*bot1) += 1;
				std::get<1>(*bot2) += 1;
			}
			if (loud) {
				std::cout << ".";
			}
			return;
		}
		board_history.push_back(new_situation);
		current_situation = board_history.rbegin();
	}
}

std::ostream& operator<<(std::ostream& stream, const Tournament& tournament) {
	stream << "Tournament:\n";
	for (auto contestant : tournament.contestants) {
		stream << std::get<0>(contestant) << " - " << std::get<1>(contestant) << "\n";
	}
	return stream;
}
