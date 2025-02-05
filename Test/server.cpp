#include "server.hpp"
#include "client_session.hpp"
#include <asio.hpp>
#include <iostream>
#include <vector>
#include <string>

// Fonction utilitaire pour récupérer les adresses IP locales (hors loopback)
std::vector<std::string> get_local_ips() {
    std::vector<std::string> ips;
    try {
        asio::io_context io_context;
        // Récupérer le nom de l'hôte de la machine
        std::string hostname = asio::ip::host_name();
        // Résoudre le nom de l'hôte pour obtenir les endpoints
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(hostname, "");
        for (const auto& entry : endpoints) {
            asio::ip::address addr = entry.endpoint().address();
            // Exclure l'adresse de loopback (127.0.0.1 ou ::1)
            if (!addr.is_loopback()) {
                ips.push_back(addr.to_string());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la récupération des IP locales : " << e.what() << std::endl;
    }
    return ips;
}

Server::Server(asio::io_context& io_context, unsigned short port)
    : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
    try {
        // Afficher l'endpoint d'écoute (souvent 0.0.0.0:port)
        asio::ip::tcp::endpoint localEndpoint = acceptor_.local_endpoint();
        std::cout << "Serveur en écoute sur " 
                  << localEndpoint.address().to_string() 
                  << ":" << localEndpoint.port() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la récupération de l'endpoint local : " << e.what() << std::endl;
    }

    // Afficher les adresses IP locales réelles (hors loopback)
    std::vector<std::string> ips = get_local_ips();
    if (!ips.empty()) {
        std::cout << "Adresses IP locales disponibles :" << std::endl;
        for (const auto& ip : ips) {
            std::cout << "  " << ip << std::endl;
        }
    } else {
        std::cout << "Aucune adresse IP locale trouvée hors loopback." << std::endl;
    }

    start_accept();
}

void Server::start_accept() {
    acceptor_.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
        if (!ec) {
            auto session = std::make_shared<ClientSession>(std::move(socket));

            // Définir un callback pour la déconnexion : si un client se déconnecte, terminer l'appel.
            session->set_on_disconnect([this](ClientSession::Ptr s) {
                std::cout << "Un client s'est déconnecté. Fin de l'appel.\n";
                if (session1_ && session2_) {
                    session1_->close();
                    session2_->close();
                    session1_.reset();
                    session2_.reset();
                }
            });

            // Affecter la première session disponible.
            if (!session1_) {
                session1_ = session;
                std::cout << "Client 1 connecté.\n";
                session1_->start([this](const std::vector<char>& data, ClientSession::Ptr /*sender*/) {
                    // Relayer les données vers le client 2 s'il est connecté.
                    if (session2_) {
                        session2_->do_write(data);
                    }
                });
            } else if (!session2_) {
                session2_ = session;
                std::cout << "Client 2 connecté.\n";
                session2_->start([this](const std::vector<char>& data, ClientSession::Ptr /*sender*/) {
                    // Relayer les données vers le client 1 s'il est connecté.
                    if (session1_) {
                        session1_->do_write(data);
                    }
                });
            } else {
                // Rejeter toute connexion supplémentaire au-delà de deux.
                std::cerr << "Connexion rejetée : déjà deux clients connectés.\n";
                session->close();
            }
        } else {
            std::cerr << "Erreur d'acceptation : " << ec.message() << "\n";
        }
        // Continuer à accepter si une session est disponible.
        if (!session1_ || !session2_) {
            start_accept();
        }
    });
}